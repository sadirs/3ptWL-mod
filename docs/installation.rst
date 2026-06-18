Installation
============

3ptWL-mod is built from source.  The standard build produces the ``wlcf``
executable, ``libwlcf.a`` static library, and ``wlcfpy`` Python extension.

Prerequisites
-------------

The native code requires:

* a C compiler, normally GCC;
* the GNU Scientific Library (GSL);
* FFTW3;
* ``make`` and ``ar``;
* OpenMP support when parallel execution is enabled.

The wrapper additionally requires Python, NumPy, Cython, setuptools, and wheel.
On Debian or Ubuntu, the native dependencies are typically installed with:

.. code-block:: bash

   sudo apt-get update
   sudo apt-get install build-essential libgsl-dev libfftw3-dev python3-dev

Clone the Repository
--------------------

.. code-block:: bash

   git clone https://github.com/sadirs/3ptWL-mod.git
   cd 3ptWL-mod

Build All Interfaces
--------------------

.. code-block:: bash

   make clean
   make PYTHON=python3 all

This command builds ``wlcf``, ``libwlcf.a``, and installs a locally compiled
``wlcfpy`` extension for the selected Python interpreter.

To build only the executable and static library:

.. code-block:: bash

   make clean
   make wlcf libwlcf.a

Custom Dependency Locations
---------------------------

The Makefiles use ``gsl-config`` and ``pkg-config`` when available and fall
back to common system locations.  Explicit include and library directories can
be supplied at build time:

.. code-block:: bash

   make PYTHON=python3 all \
      GSL_INCLUDE=/path/to/gsl/include GSL_LIB=/path/to/gsl/lib \
      FFTW3_INCLUDE=/path/to/fftw/include FFTW3_LIB=/path/to/fftw/lib

For the Python extension, ``setup.py`` also recognizes ``GSL_DIR``,
``FFTW_DIR``, ``WLCF_INCLUDE_DIRS``, and ``WLCF_LIBRARY_DIRS``.

Verify the Installation
-----------------------

Check the compiled parameter interface:

.. code-block:: bash

   ./wlcf --help

Check the Python extension:

.. code-block:: bash

   python3 -c "from wlcfpy import wlcf; print(wlcf)"

Then run the reduced calculation in :doc:`quickstart`.

Build the Documentation
-----------------------

.. code-block:: bash

   python3 -m pip install -r docs/requirements.txt
   make -C docs html

The rendered site is written to ``docs/_build/html``.
