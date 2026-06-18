Outputs and File Formats
========================

Every run writes beneath ``rootDir``.  Model, grid, and summary files use the
configured ``prefix`` where applicable.

Run Metadata
------------

``<rootDir>/*-usedvalues``
   Complete parameter values used by the run.

``<rootDir>/tmp/wlcf.log``
   Log output when ``verbose_log`` is greater than zero.

``<rootDir>/<prefix>info.txt``
   Compact summary of cosmology and numerical-grid settings.

Background and Grid Tables
--------------------------

``<rootDir>/<prefix>background_functions.txt``
   Radial table containing redshift, distance, growth, nonlinear-scale,
   effective-index, and lensing-weight quantities.

``<rootDir>/<prefix>theta_array.txt``
   Angular separation grid for the 3PCF multipoles.

``<rootDir>/<prefix>ellArray.txt``
   Angular multipole grid for projected bispectrum products.

``<rootDir>/<prefix>kArray.txt``
   Wavenumber grid for Fourier-space bispectrum products.

3PCF and Bispectrum Tables
--------------------------

``<rootDir>/<prefix>zetam<m>.txt``
   Matrix of :math:`\zeta_m(\theta_1,\theta_2)` for each moment from zero
   through ``mMax``.

``<rootDir>/<prefix>Bmells_<m>.txt``
   Projected bispectrum multipole matrix on the ``ellArray`` grid.

``<rootDir>/<prefix>BmellsVector_<m>.txt``
   Flattened projected-bispectrum matrix written when ``writevectors`` is true.

``<rootDir>/<prefix>Bnk_<m>.txt``
   Fourier-space bispectrum matrix on the ``kArray`` grid.

Production Practice
-------------------

Use a distinct ``rootDir`` or ``prefix`` for every configuration.  Keep the
used-parameter file and ``info.txt`` with the numerical products.  Before a
large scan, verify expected shapes and finite values with a reduced run such as
the one in :doc:`quickstart`.
