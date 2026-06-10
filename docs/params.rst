Parameters
==========

This page documents the main parameters used by ``wlcf`` for weak-lensing
3PCF calculations. The compiled source of truth is
``getparam/cmdline_defs.h``. For the exact list available in your build, run:

.. code-block:: bash

    ./wlcf --help

When running from ``tests``, use:

.. code-block:: bash

    ../wlcf --help

Command-line parameters use ``key=value`` with no spaces around ``=``.
Parameter files use the same names and may include spaces around ``=``.

Parameter Files
---------------

:paramfile: Optional parameter file. Parameters in the file override compiled
    defaults. Command-line values can then override file values.

Example:

.. code-block:: bash

    ../wlcf paramfile=my_run.ini tree_level=4 verb=2

After each run, ``wlcf`` writes a ``*-usedvalues`` file in ``rootDir``. This is
the safest template for a new parameter file because it records all values used
by that build.

Cosmology
---------

:z: Redshift used in the computation. Default: ``1.0334``.

:h: Dimensionless Hubble parameter. Default: ``0.7``.

:sigma8: Normalization of the matter power spectrum. Alias: ``s8``. Default:
    ``0.82``.

:Omb: Baryon density parameter. Default: ``0.046``.

:Omc: Cold dark matter density parameter. Default: ``0.233``.

:Omnu: Massive neutrino density parameter. Default: ``0.0``.

:ns: Spectral index of the linear power spectrum. Default: ``0.97``.

:w: Dark energy equation-of-state parameter. Default: ``-1.0``.

Input Files
-----------

:fnamePS: Linear matter power spectrum table. Alias: ``ps``. Default:
    ``./input/linear_pk_Takahashi_z0.txt``.

The power-spectrum file is a plain numeric two-column ASCII table:
``k`` and ``P(k)``. See :doc:`io_formats`.

:Wg: Weak-lensing kernel mode. Default: ``0``.

    * ``0`` uses a Dirac-delta source at the redshift bin.
    * ``1`` reads a kernel from ``fWgchi``.

:fWgchi: Weak-lensing kernel table. Alias: ``fwgchi``. Default:
    ``./input/Wg_Takahashi_z05078.txt``.

Model And Numerical Setup
-------------------------

:tree_level: Bispectrum/model branch. Alias: ``tlev``. Default: ``3``.

    * ``1`` = Standard Perturbation Theory (SPT).
    * ``2`` = P2 approximation.
    * ``3`` = EFT branch.
    * ``4`` or larger = Takahashi/Halo-model inspired branch.

:zbin: Redshift bin used in the projection. Default: ``0.5078``.

:mMax: Maximum multipole order. The code writes moments from ``0`` through
    ``mMax``. Default: ``5``.

:chiQuadSteps: Number of radial integration steps. Alias: ``chiqst``.
    Default: ``300``.

:GLpoints: Number of Gauss-Legendre points. Alias: ``gl``. Default: ``64``.

:Nell: Number of angular multipole samples. Default: ``128``.

:ellmin: Minimum multipole value. Default: ``0.001``.

:ellmax: Maximum multipole value. Default: ``10000.0``.

Output
------

:rootDir: Output directory for logs and used-parameter files. Alias: ``root``.
    Default: ``Output``.

:path_Bells: Output directory for angular correlation and bispectrum tables.
    Alias: ``bellout``. Default: ``Bell_outputs``.

:prefix: Prefix prepended to most output file names. Alias: ``pre``. Default:
    ``run1_``.

:writevectors: Whether to write flattened intermediate vectors. Default:
    ``true``.

Avoid leading ``./`` and trailing ``/`` in local ``rootDir`` values; the C
startup code creates missing output directories from the provided path.

Verbosity And Runtime
---------------------

:chatty: Additional internal progress output. Default: ``2``.

:verbose: Terminal verbosity. Alias: ``verb``. Default: ``2``.

:verbose_log: Log-file verbosity. Alias: ``verblog``. Default: ``1``.

:numberThreads: Number of OpenMP threads when OpenMP support is compiled.
    Alias: ``nthreads``. Default in OpenMP builds: ``16``.

:options: Additional option string for specialized behavior. Alias: ``opt``.
    Default: empty.

Examples
--------

Run the default parameter set from ``tests``:

.. code-block:: bash

    ../wlcf

Run the Takahashi/Halo-model branch:

.. code-block:: bash

    ../wlcf tree_level=4 ps=./input/linear_pk_Takahashi_z0.txt

Run with a custom output prefix:

.. code-block:: bash

    ../wlcf prefix=halo_model_z05078_ path_Bells=Bell_outputs
