"""
.. module:: wlcfpy
    :synopsis: Python wrapper around wlcf
.. moduleauthor:: Mario A. Rodriguez-Meza <marioalberto.rodriguezmeza@gmail.com>

.. based on Julien Lesgourges' CLASS

This module defines a class called wlcf.

# MAR 15.02.2026: TODO:

"""

import numpy as np
cimport numpy as np
from libc.stdlib cimport *
from libc.stdio cimport *
from libc.string cimport *
import cython
cimport cython

import time

import sys
def viewdictitems(d):
    if sys.version_info >= (3,0):
        return d.items()
    else:
        return d.viewitems()

ctypedef np.float64_t DTYPE_t
ctypedef np.int32_t DTYPE_i

from cwlcfpy cimport *

class cBallsError(Exception):
    def __init__(self, message=""):
        self.message = message.decode() if isinstance(message,bytes) else message

    def __str__(self):
        return '\n\nError in wlcf: ' + self.message

class cBallsSevereError(cBallsError):
    """
    Raised when wlcf failed to understand one or more input parameters.

    This case would not raise any problem in wlcf default behaviour. However,
    for parameter extraction, one has to be sure that all input parameters were
    understood, otherwise the wrong computation would be selected.
    """
    pass

class cBallsSevereErrorDummy():
    """
    Raised when wlcf failed to understand one or more input parameters.

    This case would not raise any problem in wlcf default behaviour. However,
    for parameter extraction, one has to be sure that all input parameters were
    understood, otherwise the wrong computation would be selected.
    """
    pass

class cBallsComputationError(cBallsError):
    """
    Raised when wlcf could not compute the computation at this point.

    """
    pass

cdef class wlcf:
    """
    wlcf wrapping, creates the glue between C and python

    The actual wlcf wrapping, the only class we will call from Python
    (indeed the only one we will import, with the command:
    from wlcfpy import wlcf

    """
    cdef cmdline_data cmd
    cdef global_data gd
    cdef file_content fc

    cdef int nthreads
    cdef double cputime

    cdef int computed # Flag to see if wlcfpy has already computed with the given pars
    cdef int allocated # Flag to see if wlcfpy structs are allocated already
    cdef object _pars # Dictionary of the parameters
    cdef object ncp   # Keeps track of the structures initialized, in view of cleaning.

    def set_default(self):
        _pars = {
            "numberThreads":16,
            "verbose":0,
            "verbose_log":0,
            }
        self.set(**_pars)

    def __cinit__(self, default=True):
        cdef char* dumc
        self.allocated = False
        self.computed = False
        self._pars = {}
        self.fc.size=0
        self.fc.filename = <char*>malloc(sizeof(char)*30)
        assert(self.fc.filename!=NULL)
        dumc = "NOFILE"
        sprintf(self.fc.filename,"%s",dumc)
        self.ncp = set()
        if default: self.set_default()

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

    def set(self,*pars,**kars):
        oldpars = self._pars.copy()
        if len(pars)==1:
            self._pars.update(dict(pars[0]))
        elif len(pars)!=0:
            raise cBallsSevereError("bad call")
        self._pars.update(kars)
        if viewdictitems(self._pars) <= viewdictitems(oldpars):
          return # Don't change the computed states, if the new dict was already contained in the previous dict
        self.computed=False
        return True

    def clean(self):
        self._pars = {}
        self.computed = False

    def _fillparfile(self):
        cdef char* dumc

        if self.fc.size!=0:
            free(self.fc.name)
            free(self.fc.value)
            free(self.fc.read)
        self.fc.size = len(self._pars)
        self.fc.name = <FileArg*> malloc(sizeof(FileArg)*len(self._pars))
        assert(self.fc.name!=NULL)

        self.fc.value = <FileArg*> malloc(sizeof(FileArg)*len(self._pars))
        assert(self.fc.value!=NULL)

        self.fc.read = <short*> malloc(sizeof(short)*len(self._pars))
        assert(self.fc.read!=NULL)

        i = 0
        for kk in self._pars:
            dumcp = kk.strip().encode()
            dumc = dumcp
            sprintf(self.fc.name[i],"%s",dumc)
            dumcp = str(self._pars[kk]).strip().encode()
            dumc = dumcp
            sprintf(self.fc.value[i],"%s",dumc)
            self.fc.read[i] = FALSE
            i+=1

    def struct_cleanup(self):
        if(self.allocated != True):
          return
#        if "MainLoop" in self.ncp:

#        self.ncp = set()

        self.allocated = False
        self.computed = False
#        self.gd.tree_allocated=self.getTreeAllocated()
#        self.gd.gd_allocated_2=self.getAllocated2()
#        self.gd.bodytable_allocated=self.getBodytableAllocated()
#        self.gd.histograms_allocated=self.getHistogramsAllocated()
#        self.gd.gd_allocated=self.getGDAllocated()
#        self.gd.cmd_allocated=self.getCMDAllocated()

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
        if "EndRun" in level:
            if "MainLoop" not in level:
                level.append("MainLoop")
        if "MainLoop" in level:
            if "Initial" not in level:
                level.append("Initial")
        if "Initial" in level:
            if "SetNumberThreads" not in level:
                level.append("SetNumberThreads")
        if "SetNumberThreads" in level:
            if "PrintParameterFile" not in level:
                level.append("PrintParameterFile")
        if "PrintParameterFile" in level:
            if "StartRun_Common" not in level:
                level.append("StartRun_Common")
        if len(level)!=0 :
            if "input" not in level:
                level.append("input")
        return level

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

################# Begin Run ####################
    def Run(self, level=["MainLoop"]):
        """
        Run(level=["MainLoop"])

        Main function, execute all routines in wlcf.

        Parameters
        ----------
        level : list
                list of the last module desired.

        .. warning::

            level default value

        """
        cdef ErrorMsg errmsg

        level = self._check_task_dependency(level)

        if self.computed and self.ncp.issuperset(level):
            return

        if self.allocated:
            self.struct_cleanup()

        self.computed = False

        self._fillparfile()

        self.ncp=set()
        self.allocated = True

        if "input" in level:
            if input_read_from_file(&self.cmd, &self.gd, &self.fc, errmsg) == FAILURE:
                raise cBallsSevereError(errmsg)
            self.ncp.add("input")
            problem_flag = False
            problematic_parameters = []
            for i in range(self.fc.size):
                if self.fc.read[i] == FALSE:
                    problem_flag = True
                    problematic_parameters.append(self.fc.name[i].decode())
            if problem_flag:
                raise cBallsSevereError(
                    "wlcf did not read input parameter(s): %s\n" % ', '.join(
                    problematic_parameters))

        if "StartRun_Common" in level:
            if StartRun_Common(&(self.cmd), &(self.gd)) == FAILURE:
                self.struct_cleanup()
                raise cBallsComputationError(self.op.error_message)
            self.ncp.add("StartRun_Common")

        if "PrintParameterFile" in level:
            if PrintParameterFile(&(self.cmd), &(self.gd), "wlcfpy_param.txt") == FAILURE:
                self.struct_cleanup()
                raise cBallsComputationError(self.op.error_message)
            self.ncp.add("PrintParameterFile")

        if "SetNumberThreads" in level:
            if SetNumberThreads(&(self.cmd)) == FAILURE:
                self.struct_cleanup()
                raise cBallsComputationError(self.op.error_message)
            self.ncp.add("SetNumberThreads")
            self.nthreads=self.getNThreads()

        if "Initial" in level:
            if Initial(&(self.cmd), &(self.gd)) == FAILURE:
                self.struct_cleanup()
                raise cBallsComputationError(self.op.error_message)
            self.ncp.add("Initial")

        if "MainLoop" in level:
            start_wall_time_p = time.process_time()
            if MainLoop(&(self.cmd), &(self.gd)) == FAILURE:
                self.struct_cleanup()
                raise cBallsComputationError(self.op.error_message)
            self.ncp.add("MainLoop")
            end_wall_time_p = time.process_time()
            self.cputime = (end_wall_time_p - start_wall_time_p)/self.nthreads

        self.computed = True

        return self.cputime
################# End Run ####################

#
#B Interfaces to PXD functions
#
#B flags

#E

#B parameters

# in common_defs.h:
#define MAXLENGTHOFFILES        1024
#

    def getNThreads(self):
        cdef int value
        cdef int out_value
        if get_nthreads(&self.cmd,&value)== FAILURE:
            raise cBallsSevereErrorDummy()
        out_value = value
        return out_value

#E parameters

#B histograms

#E histograms

#
#E Interfaces to PXD functions
#
