"""
.. module:: wlcfpy
    :synopsis: Python wrapper around wlcf
.. moduleauthor:: Mario A. Rodriguez-Meza <marioalberto.rodriguezmeza@gmail.com>

.. based on Julien Lesgourges' CLASS

This module defines a class called wlcf.

# MAR 15.02.2026:

"""

from math import exp,log
import numpy as np
from os.path import abspath, dirname
cimport numpy as np
from libc.stdlib cimport *
from libc.stdio cimport *
from libc.string cimport *
import cython
cimport cython
from scipy.interpolate import CubicSpline
from scipy.interpolate import UnivariateSpline
from scipy.interpolate import interp1d

import time

import sys
def viewdictitems(d):
    if sys.version_info >= (3,0):
        return d.items()
    else:
        return d.viewitems()

ctypedef np.float64_t DTYPE_t
ctypedef np.int32_t DTYPE_i

from libc.stdio cimport snprintf
from libc.stddef cimport size_t

class CallableFloat(float):
  def __call__(self):
    return self


# Import the .pxd containing definitions
from cwlcfpy cimport *

DEF _MAXTITLESTRINGLENGTH_ = 8000

__version__ = _VERSION_.decode("utf-8")

class CosmoError(Exception):
    def __init__(self, message=""):
        self.message = message.decode() if isinstance(message,bytes) else message

    def __str__(self):
        return '\n\nError in wlcf: ' + self.message


class CosmoSevereError(CosmoError):
    """
    Raised when wlcf failed to understand one or more input parameters.

    This case would not raise any problem in wlcf default behaviour. However,
    for parameter extraction, one has to be sure that all input parameters were
    understood.
    """
    pass

class CosmoSevereErrorDummy():
    """
    Raised when wlcf failed to understand one or more input parameters.

    This case would not raise any problem in wlcf default behaviour. However,
    for parameter extraction, one has to be sure that all input parameters were
    understood.
    """
    pass

class CosmoComputationError(CosmoError):
    """
    Raised when wlcf could not compute at this point.

    This will be caught by the parameter extraction code to give an extremely
    unlikely value to this point
    """
    pass

cdef inline void safe_copy_cstr(char *dest, size_t dest_size, bytes value, str label) except *:
    cdef size_t value_len = len(value)

    if value_len >= dest_size:
        raise CosmoSevereError(
            f"{label} is too long: {value_len} bytes, max is {dest_size - 1}"
        )

    if b"\0" in value:
        raise CosmoSevereError(
            f"{label} contains an embedded NUL byte"
        )

    memcpy(dest, <const char *> value, value_len)
    dest[value_len] = 0

cdef class wlcf:
    """
    wlcf wrapping, creates the glue between C and python

    The actual wlcf wrapping, the only class we will call from Python
    (indeed the only one we will import, with the command:
    from wlcfpy import wlcf

    """
    # List of used structures, defined in the header file. They have to be
    # "cdefined", because they correspond to C structures
    cdef file_content fc
    cdef cmdline_data cmd
    cdef global_data gd

    cdef ErrorMsg error_message

    cdef int nthreads
    cdef double cputime

    cdef int computed
    cdef int allocated
    cdef object _pars
    cdef object ncp

    cdef char path_to_this[1000]

    _levellist = ["input","StartRun_Common","PrintParameterFile","SetNumberThreads","Initial", "MainLoop", "EndRun"]

    # Special properties
    @property
    def pars(self):
      return self._pars
    @property
    def state(self):
      return True

#B definition for abi useful check

    cdef void _check_abi(self) except *:
        cdef size_t c_cmd_size = sizeof_cmdline_data()
        cdef size_t c_gd_size = sizeof_global_data()

        if sizeof(cmdline_data) != c_cmd_size:
            raise CosmoSevereError(
                "ABI mismatch for cmdline_data: "
                f"Cython sees {sizeof(cmdline_data)} bytes, "
                f"C library sees {c_cmd_size} bytes"
            )

        if sizeof(global_data) != c_gd_size:
            raise CosmoSevereError(
                "ABI mismatch for global_data: "
                f"Cython sees {sizeof(global_data)} bytes, "
                f"C library sees {c_gd_size} bytes"
            )
#E

    # Now we can start with the actual code describing the wlcf

    def set_default(self):
        _pars = {
            "numberThreads":1,}
        self.set(**_pars)

    def __cinit__(self, default=False):
        memset(&self.cmd, 0, sizeof(cmdline_data))
        memset(&self.gd, 0, sizeof(global_data))
        memset(&self.fc, 0, sizeof(file_content))

        self.allocated = False
        self.computed = False
        self._pars = {}
        self.gd.startrun_cputime = False

        self._check_abi()

        self.fc.filename = <char*> malloc(sizeof(char) * 30)
        if self.fc.filename == NULL:
            raise CosmoSevereError("not enough memory allocating fc.filename")

        dumc = "NOFILE"
        safe_copy_cstr(self.fc.filename, 30, b"NOFILE", "fc.filename")
        self.ncp = set()
        if default: self.set_default()
        try:
          import importlib.resources
          resource_path = abspath(importlib.resources.files('wlcfpy'))
        except (ImportError, AttributeError, TypeError):
          resource_path = dirname(abspath(__file__))
        path_to_this_as_bytes = resource_path.encode()
        safe_copy_cstr(self.path_to_this, sizeof(self.path_to_this),
               path_to_this_as_bytes, "path_to_this")

    def __dealloc__(self):
        if self.allocated:
          self.struct_cleanup()
        self.clean()
        # Reset all the fc to zero if its not already done
        if self.fc.size !=0:
            self.fc.size=0
            free(self.fc.name)
            free(self.fc.value)
            free(self.fc.read)
        free(self.fc.filename)

    # Set up the dictionary
    def set(self,*pars,**kars):
        oldpars = self._pars.copy()
        if len(pars)==1:
            self._pars.update(dict(pars[0]))
        elif len(pars)!=0:
            raise CosmoSevereError("bad call")
        self._pars.update(kars)
        if viewdictitems(self._pars) <= viewdictitems(oldpars):
          return # Don't change the computed states, if the new dict was already contained in the previous dict
        self.computed=False
        return True

    def clean(self):
        self._pars = {}
        self.computed = False

    # Create an equivalent of the parameter file. Non specified values will be
    # taken at their default (in wlcf)
    def _fillparfile(self):
        cdef int new_size
        cdef int i
        cdef FileArg *new_name = NULL
        cdef FileArg *new_value = NULL
        cdef short *new_read = NULL

        if self.fc.size != 0:
            free(self.fc.name)
            free(self.fc.value)
            free(self.fc.read)
            self.fc.name = NULL
            self.fc.value = NULL
            self.fc.read = NULL
            self.fc.size = 0

        new_size = len(self._pars)+1 if 'base_path' not in self._pars else len(self._pars)

        try:
            new_name = <FileArg*> malloc(sizeof(FileArg) * new_size)
            if new_name == NULL:
                raise CosmoSevereError("not enough memory allocating fc.name")

            new_value = <FileArg*> malloc(sizeof(FileArg) * new_size)
            if new_value == NULL:
                raise CosmoSevereError("not enough memory allocating fc.value")

            new_read = <short*> malloc(sizeof(short) * new_size)
            if new_read == NULL:
                raise CosmoSevereError("not enough memory allocating fc.read")

            i = 0
            for kk in self._pars:
                dumcp = kk.strip().encode()
                safe_copy_cstr(<char *> new_name[i], sizeof(FileArg), dumcp, "parameter name")

                dumcp = str(self._pars[kk]).strip().encode()
                safe_copy_cstr(<char *> new_value[i], sizeof(FileArg), dumcp, "parameter value")

                new_read[i] = FALSE
                i += 1

            if 'base_path' not in self._pars:
                safe_copy_cstr(<char *> new_name[i], sizeof(FileArg), b"base_path", "parameter name")
                safe_copy_cstr(
                    <char *> new_value[i],
                    sizeof(FileArg),
                    (<char *> self.path_to_this)[:strlen(self.path_to_this)],
                    "base_path"
                )
                new_read[i] = FALSE

            self.fc.name = new_name
            self.fc.value = new_value
            self.fc.read = new_read
            self.fc.size = new_size

        except Exception:
            if new_name != NULL:
                free(new_name)
            if new_value != NULL:
                free(new_value)
            if new_read != NULL:
                free(new_read)
            raise

    # Called at the end of a run, to free memory
    def struct_cleanup(self):
        if(self.allocated != True):
            return

        if self.gd.outlog != NULL:
            fclose(self.gd.outlog)
            self.gd.outlog = NULL

        if self.gd.tables_allocated:
            EndRun_FreeMemory_tables(&(self.cmd), &(self.gd))
        if self.gd.gd_allocated:
            EndRun_FreeMemory_gd(&(self.cmd), &(self.gd))
        if self.gd.cmd_allocated:
            EndRun_FreeMemory_cmd(&(self.cmd), &(self.gd))

        self.ncp = set()
        self.allocated = False
        self.computed = False
        

    def clean_all(self):
        self.struct_cleanup()
        self.clean()

    def _check_task_dependency(self, level):
        """
        Fill the level list with all the needed modules

        .. warning::

            the ordering of modules is obviously dependent on CLASS module order
            in the main.c file. This has to be updated in case of a change to
            this file.

        Parameters
        ----------

        level : list
            list of strings, containing initially only the last module required.
            For instance, to recover all the modules, the input should be
            ['lensing']

        """
        # If it's a string only, treat as a list
        if isinstance(level, str):
          level=[level]
        # For each item in the list
        levelset = set()
        for item in level:
          # If the item is not in the list of allowed levels, make error message
          if item not in self._levellist:
            raise CosmoSevereError("Unknown computation level: '{}'".format(item))
          # Otherwise, add to list of levels up to and including the specified level
          levelset.update(self._levellist[:self._levellist.index(item)+1])
        return levelset

    def _pars_check(self, key, value, contains=False, add=""):
        val = ""
        if key in self._pars:
            val = self._pars[key]
            if contains:
                if value in val:
                    return True
            else:
                if value==val:
                    return True
        if add:
            sep = " "
            if isinstance(add,str):
                sep = add

            if contains and val:
                    self.set({key:val+sep+value})
            else:
                self.set({key:value})
            return True
        return False

    def Run(self, level=["EndRun"]):
        """
        Run(level=["EndRun"])

        Main function, execute all the methods for all desired modules.
        This is called in Python, and this ensures that the wlcf instance
        of this class contains all the relevant quantities. Then, one can deduce
        integral quantities, etc...

        Parameters
        ----------
        level : list
                list of the last module desired. The internal function
                _check_task_dependency will then add to this list all the
                necessary modules to compute in order to initialize this last
                one. The default last module is "lensing".

        .. warning::

            level default value should be left as an array (it was creating
            problem when casting as a set later on, in _check_task_dependency)

        """
        cdef ErrorMsg errmsg

        # Append to the list level all the modules necessary to compute.
        level = self._check_task_dependency(level)

        # Check if this function ran before (self.computed should be true), and
        # if no other modules were requested, i.e. if self.ncp contains (or is
        # equivalent to) level. If it is the case, simply stop the execution of
        # the function.
        if self.computed and self.ncp.issuperset(level):
            return self.cputime

        # Check if already allocated to prevent memory leaks
        if self.allocated:
            self.struct_cleanup()

        # Otherwise, proceed with the normal computation.
        self.computed = False

        # Equivalent of writing a parameter file
        self._fillparfile()

        # self.ncp will contain the list of computed modules (under the form of
        # a set, instead of a python list)
        self.ncp=set()

#B correction
        # Up until the empty set, all modules are allocated
        # (And then we successively keep track of the ones we allocate additionally)
        self.allocated = True

        try:
            # --------------------------------------------------------------------
            # Check the presence for all wlcf modules in the list 'level'. If a
            # module is found in level, execute its routine.
            # --------------------------------------------------------------------
            # The input module should raise a CosmoSevereError, because
            # non-understood parameters asked to the wrapper is a problematic
            # situation.
            if "input" in level:
                if input_read_from_file(&self.cmd, &self.gd, &self.fc, errmsg) == FAILURE:
                    raise CosmoSevereError(errmsg)
                self.ncp.add("input")

                problem_flag = False
                problematic_parameters = []
                for i in range(self.fc.size):
                    if self.fc.read[i] == FALSE:
                        problem_flag = True
                        problematic_parameters.append((<char *> self.fc.name[i]).decode("utf-8"))

                if problem_flag:
                    raise CosmoSevereError(
                        "wlcf did not read input parameter(s): %s\n" %
                        ', '.join(problematic_parameters)
                    )

            # The following list of computation is straightforward. If the "_init"
            # methods fail, call `struct_cleanup` and raise a CosmoComputationError
            # with the error message from the faulty module of CLASS.
            if "StartRun_Common" in level:
                if StartRun_Common(&(self.cmd), &(self.gd)) == FAILURE:
                    raise CosmoComputationError((<char *> self.cmd.error_message).decode("utf-8", "replace"))
                self.ncp.add("StartRun_Common")

            # keep the rest of the C stages here too

            if "PrintParameterFile" in level:
                if PrintParameterFile(&(self.cmd), &(self.gd), "wlcfpy_param.txt") == FAILURE:
                    raise CosmoComputationError((<char *> self.cmd.error_message).decode("utf-8", "replace"))
                self.ncp.add("PrintParameterFile")

            if "SetNumberThreads" in level:
                if SetNumberThreads(&(self.cmd)) == FAILURE:
                    raise CosmoComputationError((<char *> self.cmd.error_message).decode("utf-8", "replace"))
                self.ncp.add("SetNumberThreads")
                self.nthreads=self.getNThreads()

            if "Initial" in level:
                if Initial(&(self.cmd), &(self.gd)) == FAILURE:
                    raise CosmoComputationError((<char *> self.cmd.error_message).decode("utf-8", "replace"))
                self.ncp.add("Initial")

            if "MainLoop" in level:
                start_wall_time_p = time.process_time()
                if MainLoop(&(self.cmd), &(self.gd)) == FAILURE:
                    raise CosmoComputationError((<char *> self.cmd.error_message).decode("utf-8", "replace"))
                self.ncp.add("MainLoop")
                end_wall_time_p = time.process_time()
                self.cputime = (end_wall_time_p - start_wall_time_p)/self.nthreads

            if "EndRun" in level:
                if EndRun(&(self.cmd), &(self.gd)) == FAILURE:
                    raise CosmoComputationError((<char *> self.cmd.error_message).decode("utf-8", "replace"))
                self.ncp.add("EndRun")
                self.allocated = False

        except Exception:
            self.struct_cleanup()
            raise
#E

        self.computed = True

        return self.cputime

        # At this point, the wlcf instance contains everything needed. The
        # following functions are only to output the desired numbers

#
#
#B wlcf definitions
#

    def abi_sizes(self):
        return {
            "cmdline_data": sizeof_cmdline_data(),
            "global_data": sizeof_global_data(),
        }

#
#B Interfaces to PXD functions
#
#B flags
#E

#B parameters

# in common_defs.h:
#
    def getNThreads(self):
        cdef int value
        if get_nthreads(&self.cmd,&value)== FAILURE:
            raise CosmoSevereErrorDummy()
        return value

#E parameters

#B gd values
#
#    def getIntegral(self):
#        cdef double value
#        if get_integral(&self.gd,&value)== FAILURE:
#            raise CosmoSevereErrorDummy()
#        return value

#E gd values

#B histograms

#E histograms

#
#E Interfaces to PXD functions
#

#E wlcf definitions
