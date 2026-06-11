"""Build the ``wlcfpy`` Cython extension.

This setup file intentionally avoids machine-specific absolute paths. It first
uses environment variables, then ``pkg-config``, and finally common system,
Conda, and Homebrew locations to find GSL, FFTW3, OpenMP, and ``libwlcf.a``.

Useful overrides:

    WLCF_LIBRARY_DIRS=/path/to/lib:/other/lib
    WLCF_INCLUDE_DIRS=/path/to/include:/other/include
    GSL_DIR=/path/to/gsl-prefix
    FFTW_DIR=/path/to/fftw-prefix
    WLCF_OPENMP=0
    WLCF_OPENMP_FLAG=-fopenmp

Before installing the Python wrapper, build the static WLCF library with:

    make libwlcf.a
"""

from __future__ import annotations

import os
import platform
import shlex
import shutil
import subprocess
from pathlib import Path
from typing import Optional, Tuple

import numpy as np
from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext as build_ext_base

try:
    from Cython.Build import cythonize
except ImportError as exc:
    raise SystemExit(
        "Cython is required to build wlcfpy. Install it with: "
        "python -m pip install cython"
    ) from exc


ROOT = Path(__file__).resolve().parent
PYTHON_DIR = ROOT / "python"
CLASS_LIB_DIR = ROOT / "addons" / "class_lib"


def split_paths(value: Optional[str]) -> list[str]:
    if not value:
        return []
    return [part for part in value.split(os.pathsep) if part]


def append_unique(items: list[str], values) -> None:
    for value in values:
        if value and value not in items:
            items.append(value)


def pkg_config(packages: list[str]) -> tuple[list[str], list[str], list[str], list[str]]:
    """Return include dirs, library dirs, libraries, and extra linker args."""
    if shutil.which("pkg-config") is None:
        return [], [], [], []

    include_dirs: list[str] = []
    library_dirs: list[str] = []
    libraries: list[str] = []
    extra_link_args: list[str] = []

    for package in packages:
        result = subprocess.run(
            ["pkg-config", "--cflags", "--libs", package],
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
        if result.returncode != 0:
            continue

        for token in shlex.split(result.stdout):
            if token.startswith("-I"):
                append_unique(include_dirs, [token[2:]])
            elif token.startswith("-L"):
                append_unique(library_dirs, [token[2:]])
            elif token.startswith("-l"):
                append_unique(libraries, [token[2:]])
            else:
                append_unique(extra_link_args, [token])

    return include_dirs, library_dirs, libraries, extra_link_args


def prefixed_dirs(env_name: str) -> tuple[list[str], list[str]]:
    prefix = os.environ.get(env_name)
    if not prefix:
        return [], []
    base = Path(prefix)
    return [str(base / "include")], [str(base / "lib")]


def gcc_runtime_library_dir() -> Optional[str]:
    compiler = shlex.split(os.environ.get("CC", "gcc"))
    try:
        result = subprocess.run(
            [*compiler, "-print-libgcc-file-name"],
            check=True,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
    except (OSError, subprocess.CalledProcessError):
        return None
    path = Path(result.stdout.strip())
    return str(path.parent) if path.name else None


def read_version() -> str:
    common_h = CLASS_LIB_DIR / "common.h"
    if not common_h.exists():
        return "0.0.0"
    for line in common_h.read_text(errors="ignore").splitlines():
        if "_VERSION_" in line:
            return line.split()[-1].strip('"').lstrip("v")
    return "0.0.0"


def env_flag(name: str, default: bool) -> bool:
    value = os.environ.get(name)
    if value is None:
        return default
    return value.strip().lower() not in {"0", "false", "no", "off"}


def openmp_flags() -> Tuple[list[str], list[str]]:
    if not env_flag("WLCF_OPENMP", platform.system() != "Darwin"):
        return [], []
    flag = os.environ.get("WLCF_OPENMP_FLAG", "-fopenmp")
    return [flag], [flag]


def fail_if_missing_static_library(library_dirs: list[str]) -> None:
    for directory in library_dirs:
        if (Path(directory) / "libwlcf.a").exists():
            return
    raise SystemExit(
        "Could not find libwlcf.a. Build it first with:\n\n"
        "    make libwlcf.a\n\n"
        "If it lives outside the repository root, set WLCF_LIBRARY_DIRS."
    )


class BuildExt(build_ext_base):
    def build_extensions(self) -> None:
        for extension in self.extensions:
            if extension.name == "wlcfpy":
                fail_if_missing_static_library(list(extension.library_dirs or []))
        super().build_extensions()


def build_extension() -> Extension:
    pkg_includes, pkg_lib_dirs, pkg_libs, pkg_extra_link = pkg_config(["gsl", "fftw3"])
    gsl_includes, gsl_lib_dirs = prefixed_dirs("GSL_DIR")
    fftw_includes, fftw_lib_dirs = prefixed_dirs("FFTW_DIR")
    conda_prefix = os.environ.get("CONDA_PREFIX")

    include_dirs = [
        np.get_include(),
        str(ROOT / "include"),
        str(CLASS_LIB_DIR),
        str(ROOT / "general_libs"),
        str(ROOT / "getparam"),
        str(ROOT / "addons" / "pxd"),
        str(ROOT / "source"),
    ]
    append_unique(include_dirs, split_paths(os.environ.get("WLCF_INCLUDE_DIRS")))
    append_unique(include_dirs, gsl_includes + fftw_includes + pkg_includes)
    append_unique(include_dirs, [
        str(Path(conda_prefix) / "include") if conda_prefix else "",
        "/usr/local/include",
        "/usr/include",
        "/opt/homebrew/include",
    ])

    library_dirs = [str(ROOT)]
    append_unique(library_dirs, split_paths(os.environ.get("WLCF_LIBRARY_DIRS")))
    append_unique(library_dirs, gsl_lib_dirs + fftw_lib_dirs + pkg_lib_dirs)
    append_unique(library_dirs, [
        gcc_runtime_library_dir(),
        str(Path(conda_prefix) / "lib") if conda_prefix else "",
        "/usr/local/lib",
        "/usr/lib",
        "/usr/lib/x86_64-linux-gnu",
        "/opt/homebrew/lib",
    ])

    libraries = ["wlcf"]
    append_unique(libraries, pkg_libs or ["gsl", "gslcblas", "fftw3"])
    append_unique(libraries, ["m"])

    compile_args, openmp_link_args = openmp_flags()
    extra_link_args = pkg_extra_link + openmp_link_args
    if platform.system() != "Darwin":
        append_unique(extra_link_args, ["-lz"])

    return Extension(
        "wlcfpy",
        [str(PYTHON_DIR / "wlcfpy.pyx")],
        include_dirs=include_dirs,
        library_dirs=library_dirs,
        libraries=libraries,
        extra_compile_args=compile_args,
        extra_link_args=extra_link_args,
    )


extension = build_extension()
extension.cython_directives = {"language_level": "3"}

setup(
    name="wlcfpy",
    version=read_version(),
    description="Python interface to the 3ptWL-mod modeling code",
    url="https://github.com/sadirs/3ptWL-mod",
    python_requires=">=3.9",
    cmdclass={"build_ext": BuildExt},
    ext_modules=cythonize([extension], compiler_directives={"language_level": "3"}),
)
