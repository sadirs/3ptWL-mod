Command-Line Usage
==================

The ``wlcf`` executable accepts ``key=value`` arguments and parameter files.
The compiled source of truth is ``include/cmdline_defs.h``; inspect the exact
interface for your build with:

.. code-block:: bash

   ./wlcf --help

Syntax
------

Pass command-line values without spaces around ``=``:

.. code-block:: bash

   ./wlcf tree_level=4 rootDir=Output_halo prefix=halo_ verbose=1

Use ``paramfile`` for a reusable configuration:

.. code-block:: bash

   ./wlcf paramfile=my_run.ini numberThreads=8

Command-line values override values read from the parameter file.  Each run
writes a complete ``*-usedvalues`` record beneath ``rootDir``; that file is the
safest template for a new configuration.

Cosmology
---------

``z``
   Evaluation redshift. Default: ``1.0334``.

``h``, ``Omb``, ``Omc``, ``Omnu``
   Dimensionless Hubble parameter and matter-density components.

``sigma8``
   Power-spectrum normalization. Alias: ``s8``.

``ns``, ``w``
   Scalar spectral index and dark-energy equation-of-state parameter.

Inputs
------

``fnamePS``
   Linear matter power-spectrum table. Alias: ``ps``.

``Wg``
   Lensing-kernel mode. ``0`` uses a source plane at ``zbin``; ``1`` reads
   ``fWgchi``.

``fWgchi``
   Two-column weak-lensing kernel table. Alias: ``fwgchi``.

See :doc:`ps_files` for file formats and unit conventions.

Model and Numerical Controls
----------------------------

``tree_level``
   Bispectrum branch. Alias: ``tlev``. ``1`` selects standard perturbation
   theory, ``2`` the power-spectrum-squared approximation, ``3`` the EFT
   branch, and ``4`` the Takahashi/Halo-model inspired branch.

``zbin``
   Source redshift used in the projected weak-lensing calculation.

``mMax``
   Maximum 3PCF multipole. Moments from zero through ``mMax`` are written.

``chiQuadSteps``
   Number of radial integration steps. Alias: ``chiqst``.

``GLpoints``
   Number of Gauss-Legendre points. Alias: ``gl``.

``Nell``, ``ellmin``, ``ellmax``
   Size and range of the angular multipole grid.

Output and Runtime
------------------

``rootDir``
   Directory containing every product from a run. Alias: ``root``. Default:
   ``Output``.

``prefix``
   Prefix prepended to model and grid products. Alias: ``pre``. Default:
   ``run1_``.

``writevectors``
   Write flattened intermediate vectors in addition to matrix products.

``verbose`` and ``verbose_log``
   Terminal and file-log verbosity. Aliases: ``verb`` and ``verblog``.

``numberThreads``
   OpenMP thread count. Alias: ``nthreads``.

``options``
   Optional specialized behavior string. Alias: ``opt``.

Example
-------

.. code-block:: bash

   ./wlcf rootDir=Output_halo prefix=halo_z05078_ \
      fnamePS=./input/linear_pk_Takahashi_z0.txt \
      tree_level=4 zbin=0.5078 mMax=5 Nell=128 \
      chiQuadSteps=300 GLpoints=64 numberThreads=8
