Python Interface
================

``wlcf`` can be driven from Python in two ways:

* call the executable with ``subprocess``;
* use the Cython wrapper exposed as ``wlcfpy``.

Calling The Executable
----------------------

The command-line interface is often the easiest option for scans and batch
workflows:

.. code-block:: python

    import subprocess

    subprocess.run(
        [
            "../wlcf",
            "tree_level=4",
            "ps=./input/linear_pk_Takahashi_z0.txt",
            "prefix=halo_model_z05078_",
        ],
        check=True,
    )

Parameters must be passed as ``key=value`` strings with no spaces around
``=``.

Cython Wrapper
--------------

Build the wrapper with:

.. code-block:: bash

    make clean
    make all

or, with a specific interpreter:

.. code-block:: bash

    PYTHON=python3 make all

Then check the import:

.. code-block:: bash

    python -c "from wlcfpy import wlcf; print(wlcf)"

Minimal wrapper usage from inside ``tests``:

.. code-block:: python

    from wlcfpy import wlcf

    w = wlcf()
    w.set(
        numberThreads=8,
        verbose=2,
        fnamePS="./input/linear_pk_Takahashi_z0.txt",
        tree_level=4,
        prefix="halo_model_z05078_",
    )
    cputime = w.Run()
    print(cputime)

The wrapper accepts either keyword arguments or a dictionary:

.. code-block:: python

    params = {
        "numberThreads": 8,
        "tree_level": 4,
        "fnamePS": "./input/linear_pk_Takahashi_z0.txt",
    }

    w = wlcf()
    w.set(params)
    w.Run()

Troubleshooting
---------------

If ``make all`` fails while building ``wlcfpy``:

* Confirm that GSL and FFTW are installed and visible through ``pkg-config`` or
  through ``GSL_DIR`` and ``FFTW_DIR``.
* Confirm that ``libwlcf.a`` exists in the repository root. If not, run
  ``make libwlcf.a``.
* For custom library locations, set ``WLCF_INCLUDE_DIRS`` and
  ``WLCF_LIBRARY_DIRS`` before running ``make all``.
* If OpenMP is enabled, make sure the compiler and Python extension link
  against the matching OpenMP runtime.

If the wrapper raises an error about unread parameters, compare the parameter
names with ``../wlcf --help``. The wrapper is stricter than the executable and
reports parameters that were not understood by the C input layer.

Notes
-----

The working directory should contain the input files referenced by relative
paths such as ``./input/linear_pk_Takahashi_z0.txt``. Output files are written
using the same ``rootDir``, ``path_Bells``, and ``prefix`` rules as the C
executable.
