3ptWL-mod Installation
======================

3ptWL-mod is a C-based implementation for computing weak-lensing correlation
function models, with optional OpenMP support and a Cython wrapper for Python
workflows. This page covers the usual build and smoke-test path.

Requirements
------------

The executable requires:

* A C compiler, usually ``gcc``.
* **GSL** when ``USEGSL = 1`` in ``Makefile_settings``.
* **FFTW3** when ``USEFFTW3ON = 1`` in ``Makefile_machine``.

The Python wrapper additionally requires:

* Python.
* ``numpy``.
* ``cython``.

The bundled ``addons/gsl`` and ``addons/fftw3`` directories include source
archives and notes, but most users should prefer system or environment-managed
installations when available.

Clone
-----

Clone the repository and enter the source tree:

.. code-block:: bash

    git clone https://github.com/sadirs/3ptWL-mod.git
    cd 3ptWL-mod

Configure
---------

Edit ``Makefile_settings`` first:

* ``USEGSL`` controls whether GSL-dependent code is compiled.
* ``GSLINTERNAL`` controls whether bundled GSL sources are used.
* ``OPENMPMACHINE`` controls whether OpenMP support is compiled.
* ``ADDONSON`` enables addon include sockets and support files.

Then edit ``Makefile_machine`` for machine-specific compiler choices:

* ``CC`` selects the compiler.
* ``PYTHON`` selects the Python interpreter used by ``make all``.

The Python wrapper setup is portable and does not require editing
``setup.py`` for ordinary Linux, Conda, Homebrew, or Colab-style
installations. It searches environment variables, ``pkg-config``, and common
library locations. For custom library prefixes, use:

.. code-block:: bash

    export GSL_DIR=/path/to/gsl
    export FFTW_DIR=/path/to/fftw
    export WLCF_INCLUDE_DIRS=/extra/include
    export WLCF_LIBRARY_DIRS=/extra/lib

Compile
-------

To build the executable and static library:

.. code-block:: bash

    make clean
    make

To build the executable, static library, and Python wrapper:

.. code-block:: bash

    make clean
    make all

To force a Python 3 interpreter:

.. code-block:: bash

    PYTHON=python3 make all

If compilation fails, first verify that GSL and FFTW are installed and visible
through ``pkg-config`` or the environment variables above. If the Python wrapper
reports that ``libwlcf.a`` is missing, run ``make libwlcf.a`` or ``make all``
from the repository root.

Smoke Test
----------

Run a default example from ``tests``:

.. code-block:: bash

    cd tests
    ../wlcf

This runs with default parameter values compiled from
``getparam/cmdline_defs.h``.

Typical outputs are:

* ``Output/`` for logs and the generated ``*-usedvalues`` parameter file.
* ``Bell_outputs/`` for 3PCF and bispectrum tables.
* ``Output/tmp/wlcf.log`` when file logging is enabled.

The generated ``*-usedvalues`` file can be copied or renamed and then edited
as a custom parameter file for future runs.

Python Wrapper Check
--------------------

After ``make all`` succeeds, check that Python can import the wrapper:

.. code-block:: bash

    python -c "from wlcfpy import wlcf; print(wlcf)"

A minimal run from inside ``tests`` looks like:

.. code-block:: python

    from wlcfpy import wlcf

    w = wlcf()
    w.set(
        numberThreads=8,
        fnamePS="./input/linear_pk_Takahashi_z0.txt",
        tree_level=4,
    )
    w.Run()

Parallel Execution
------------------

OpenMP is available only when ``OPENMPMACHINE = 1`` is set before compiling.
The run-time thread count can be controlled with either the environment or the
``numberThreads`` parameter:

.. code-block:: bash

    export OMP_NUM_THREADS=8
    ../wlcf numberThreads=8

HPC Example
-----------

On NERSC Perlmutter, a typical interactive workflow is:

.. code-block:: bash

    module load gcc
    salloc -N 1 -C cpu -q interactive -t 01:00:00
    export OMP_NUM_THREADS=32
    cd 3ptWL-mod/tests
    ../wlcf numberThreads=32

Use compute nodes rather than login nodes for production-size runs.
