#
# wlcfpy python module setup
# written by: Mario A. Rodriguez-Meza
# date: 15.02.2026

from setuptools import setup, Extension

from Cython.Distutils import build_ext as cython_build_ext

import numpy as nm
import os
import subprocess
import subprocess as sbp
import os.path as osp
import sys

#B to parse some constants from C headers
import re

def read_define_int(header_path, macro_name):
    pattern = re.compile(r"^\s*#\s*define\s+" + re.escape(macro_name) + r"\s+(.+?)(?:\s*/[/*].*)?$")

    with open(header_path, "r") as f:
        for line in f:
            m = pattern.match(line)
            if not m:
                continue

            value = m.group(1).strip()
            value = value.split("//", 1)[0].strip()
            value = value.split("/*", 1)[0].strip()

            try:
                return int(value)
            except ValueError:
                raise RuntimeError(
                    f"Macro {macro_name} in {header_path} is not a plain integer: {value}"
                )

    raise RuntimeError(f"Macro {macro_name} not found in {header_path}")
#E

def first_existing_fftw_path(kind):
    candidates = [
        "/opt/homebrew",
        "/usr/local",
        os.path.join(os.path.expanduser("~"), "local", "fftw3"),
        os.path.join(os.path.expanduser("~"), "NagBody_pkg", "local", "fftw3"),
    ]

    for base in candidates:
        if kind == "include":
            path = os.path.join(base, "include")
            if os.path.exists(os.path.join(path, "fftw3.h")):
                return path
        elif kind == "lib":
            path = os.path.join(base, "lib")
            if os.path.isdir(path):
                for name in os.listdir(path):
                    if name.startswith("libfftw3."):
                        return path
    return None

def read_makefile_variable(path, key, default=None):
    try:
        with open(path, "r") as f:
            for line in f:
                line = line.strip()
                if line.startswith(key) and "=" in line:
                    return line.split("=", 1)[1].strip()
    except FileNotFoundError:
        pass
    return default


def get_gsl_config(flag):
    try:
        res = subprocess.check_output(["gsl-config", flag], text=True)
        return res.strip()
    except (FileNotFoundError, subprocess.CalledProcessError):
        return ""

GCCPATH_STRING = sbp.Popen(
    ['gcc', '-print-libgcc-file-name'],
    stdout=sbp.PIPE
).communicate()[0]
GCCPATH = osp.normpath(osp.dirname(GCCPATH_STRING)).decode()

root_folder = os.path.dirname(os.path.abspath(__file__))
include_folder = os.path.join(root_folder, "include")
general_lib_folder = os.path.join(root_folder, "general_lib")
getparam_folder = os.path.join(root_folder, "getparam")
source_folder = os.path.join(root_folder, "source")
class_lib_folder = os.path.join(root_folder, "addons", "class_lib")
pxd_folder = os.path.join(root_folder, "addons", "pxd")
wlcfpy_folder = os.path.join(root_folder, "python")
#
addons_folder = os.path.join(root_folder, "addons")
addons_include_folder = os.path.join(addons_folder, "addons_include")
addons_include_include_folder = os.path.join(addons_include_folder, "include")
addons_include_addons_folder = os.path.join(addons_include_folder, "addons")
addons_include_startrun_folder = os.path.join(addons_include_folder, "source", "startrun")
addons_include_wlcf_folder = os.path.join(addons_include_folder, "source", "wlcf")
addons_include_wlcfio_folder = os.path.join(addons_include_folder, "source", "wlcfio")
#

makefile_settings = os.path.join(root_folder, "Makefile_settings")
addons_settings = os.path.join(root_folder, "addons", "Makefile_addons_settings")

makefile_machine = os.path.join(root_folder, "Makefile_machine")

if "CC" not in os.environ:
    make_cc = read_makefile_variable(makefile_machine, "CC", None)
    if make_cc:
        os.environ["CC"] = make_cc


def read_makefile_setting(path, key, default="0"):
    try:
        with open(path, "r") as f:
            for line in f:
                line = line.strip()
                if line.startswith(key):
                    return line.split("=", 1)[1].strip()
    except FileNotFoundError:
        pass
    return os.environ.get(key, default)

#B gsl definition
def parse_gsl_config():
    cflags = get_gsl_config("--cflags").split()
    libs_flags = get_gsl_config("--libs").split()

    include_dirs = [
        flag[2:] for flag in cflags
        if flag.startswith("-I") and len(flag) > 2
    ]

    library_dirs = [
        flag[2:] for flag in libs_flags
        if flag.startswith("-L") and len(flag) > 2
    ]

    libraries = [
        flag[2:] for flag in libs_flags
        if flag.startswith("-l") and len(flag) > 2
    ]

    return include_dirs, library_dirs, libraries
#E

#B FFTW3
def parse_pkg_config(package):
    cflags = get_pkg_config(package, "--cflags").split()
    libs_flags = get_pkg_config(package, "--libs").split()

    include_dirs = [x[2:] for x in cflags if x.startswith("-I")]
    library_dirs = [x[2:] for x in libs_flags if x.startswith("-L")]
    libraries = [x[2:] for x in libs_flags if x.startswith("-l")]

    return include_dirs, library_dirs, libraries


def get_pkg_config(package, flag):
    try:
        return subprocess.check_output(
            ["pkg-config", flag, package],
            text=True
        ).strip()
    except (FileNotFoundError, subprocess.CalledProcessError):
        return ""
#E

#B GSL...
gsl_include_dirs, gsl_library_dirs, gsl_libraries = parse_gsl_config()

if os.environ.get("GSL_INCLUDE"):
    gsl_include_dirs = [os.environ["GSL_INCLUDE"]]

if os.environ.get("GSL_LIB"):
    gsl_library_dirs = [os.environ["GSL_LIB"]]

if not gsl_include_dirs:
    gsl_include_dirs = ["/usr/local/include"]

if not gsl_library_dirs:
    gsl_library_dirs = ["/usr/local/lib"]

if not gsl_libraries:
    gsl_libraries = ["gsl", "gslcblas", "m"]

USEGSL = read_makefile_setting(makefile_settings, "USEGSL", "1")
if USEGSL != "1":
    raise RuntimeError("wlcf requires USEGSL=1")

liblist = ["wlcf"] + gsl_libraries

if sys.platform.startswith("linux"):
    liblist += ["mvec"]
#E

#B FFTW3 definitions
fftw_include_dirs, fftw_library_dirs, fftw_libraries = parse_pkg_config("fftw3")

if os.environ.get("FFTW3_INCLUDE"):
    fftw_include_dirs = [os.environ["FFTW3_INCLUDE"]]

if os.environ.get("FFTW3_LIB"):
    fftw_library_dirs = [os.environ["FFTW3_LIB"]]

if not fftw_include_dirs:
    fftw_include = first_existing_fftw_path("include")
    if fftw_include:
        fftw_include_dirs = [fftw_include]

if not fftw_library_dirs:
    fftw_lib = first_existing_fftw_path("lib")
    if fftw_lib:
        fftw_library_dirs = [fftw_lib]

if not fftw_libraries:
    fftw_libraries = ["fftw3"]
#E

#B OpenMP definitions
OPENMPMACHINE = read_makefile_setting(
    os.path.join(root_folder, "Makefile_settings"),
    "OPENMPMACHINE",
    default="0",
)

openmp_compile_args = []
openmp_link_args = []

if OPENMPMACHINE == "1":
    openmp_compile_args += ["-fopenmp"]
    openmp_link_args += ["-fopenmp", "-lgomp"]
#E

#B definitions addons macros
ADDONSON = read_makefile_setting(makefile_settings, "ADDONSON", "0")
CLASSLIBON = read_makefile_setting(addons_settings, "CLASSLIBON", "0")
if CLASSLIBON == "1":
    PXDON = read_makefile_setting(addons_settings, "PXDON", "0")
else:
    PXDON = "0"

define_macros = [
    ("__WLCFDIR__", f'"{root_folder}"'),
]

define_macros.append(("USEGSL", None))

if ADDONSON == "1":
    define_macros.append(("ADDONS", None))

    if CLASSLIBON == "1":
        define_macros.append(("CLASSLIB", None))

    if PXDON == "1":
        define_macros.append(("PXD", None))
#E


#B generator function
def indent_block(text, spaces=8):
    prefix = " " * spaces
    return "\n".join(prefix + line if line.strip() else line
                     for line in text.strip("\n").splitlines())

#
# Parse C ABI constants used to generate the Cython declarations.
#
common_defs_h = os.path.join(include_folder, "common_defs.h")
global_data_h = os.path.join(include_folder, "global_data.h")
class_common_h = os.path.join(class_lib_folder, "common.h")
parser_h = os.path.join(class_lib_folder, "parser.h")

filearg_size = read_define_int(parser_h, "_ARGUMENT_LENGTH_MAX_")
errormsg_size = read_define_int(class_common_h, "_ERRORMSGSIZE_")
filename_size = (
    read_define_int(class_common_h, "_FILENAMESIZE_")
    + read_define_int(class_common_h, "_BASEPATHSIZE_")
)

PXD_CONSTANTS = {
    "FILEARG_SIZE": str(filearg_size),
    "ERRORMSG_SIZE": str(errormsg_size),
    "FILENAME_SIZE": str(filename_size),
    "FNAMEPSPATH_SIZE": str(read_define_int(global_data_h, "MAXLENGTHOFPSPATH")),
    "N_DATA_MAX": str(read_define_int(global_data_h, "n_data_max")),
    "N_CHI_DATA_MAX": str(read_define_int(global_data_h, "n_chi_data_max")),
    "MAXLENGTHOFFILES": str(read_define_int(common_defs_h, "MAXLENGTHOFFILES")),
}

#B make this correction in setup.py
#      so cwlcfpy.pxd.in produce it
#ifndef CLASSLIB
#    ErrorMsg error_message;
#endif
#E
def generate_cwlcfpy_pxd():
    template_path = os.path.join(wlcfpy_folder, "cwlcfpy.pxd.in")
    output_path = os.path.join(wlcfpy_folder, "cwlcfpy.pxd")

    cmdline_classlib_fields = ""
    cmdline_cosmolib_fields = ""
    global_classlib_fields = ""
    global_cosmolib_fields = ""

    if ADDONSON == "1" and CLASSLIBON == "1":
        cmdline_classlib_fields = indent_block("""
char base_path[1000]
ErrorMsg error_message
""")

        global_classlib_fields = indent_block("""
ErrorMsg error_message
""")

    if ADDONSON == "1":
        cmdline_cosmolib_fields = indent_block("""
""")

        global_cosmolib_fields = indent_block("""
""")

    with open(template_path, "r") as f:
        text = f.read()

    text = text.replace("{{CMDLINE_CLASSLIB_FIELDS}}", cmdline_classlib_fields)
    text = text.replace("{{CMDLINE_COSMOLIB_FIELDS}}", cmdline_cosmolib_fields)
    text = text.replace("{{GLOBAL_CLASSLIB_FIELDS}}", global_classlib_fields)
    text = text.replace("{{GLOBAL_COSMOLIB_FIELDS}}", global_cosmolib_fields)

    for key, value in PXD_CONSTANTS.items():
        text = text.replace("{{" + key + "}}", value)

    with open(output_path, "w") as f:
        f.write(text)

# This makes the .pxd match the active C macros.
#   But your wlcfpy.pyx also accesses addon-dependent fields/functions.
#   So if you want to support CLASSLIBON=0 or PXDON=0,
#   you must also guard Python wrapper code that assumes those features exist.
if ADDONSON != "1" or CLASSLIBON != "1" or PXDON != "1":
    raise RuntimeError("wlcfpy requires ADDONSON=1, CLASSLIBON=1, PXDON=1")
#
#E generator function


with open(os.path.join(class_lib_folder, 'common.h'), 'r') as v_file:
    for line in v_file:
        if line.find("_VERSION_") != -1:
            VERSION = line.split()[-1][2:-1]
            break

wlcfpy_ext = Extension(
    "wlcfpy",
        [
            os.path.join(wlcfpy_folder, "wlcfpy.pyx"),
            os.path.join(source_folder, "abi_check.c"),
        ],
    include_dirs=[
        nm.get_include(),
        include_folder,
        class_lib_folder,
        general_lib_folder,
        getparam_folder,
        pxd_folder,
        source_folder,
        #
        addons_folder,
        addons_include_folder,
        addons_include_include_folder,
        addons_include_addons_folder,
        addons_include_startrun_folder,
        addons_include_wlcf_folder,
        addons_include_wlcfio_folder,
        #
        *gsl_include_dirs,
        *fftw_include_dirs,
    ],
    define_macros=define_macros,
    libraries=liblist + fftw_libraries,
    library_dirs=[
        root_folder,
        GCCPATH,
        *gsl_library_dirs,
        *fftw_library_dirs,
    ],
    extra_compile_args=openmp_compile_args,
    extra_link_args=openmp_link_args + ['-lz'],
)

wlcfpy_ext.cython_directives = {
    'language_level': "3" if sys.version_info.major >= 3 else "2"
}

class build_ext(cython_build_ext):
    def run(self):
        generate_cwlcfpy_pxd()
        subprocess.check_call(["make", "libwlcf.a"], cwd=root_folder)
        super().run()

setup(
    name='wlcfpy',
    version=VERSION,
    description='Python interface to the covariance code wlcf',
    url='http://github.com/rodriguezmeza/wlcf.git',
    cmdclass={'build_ext': build_ext},
    ext_modules=[wlcfpy_ext],
)
