Preparation and Post-Processing
===============================

Pre-Processing
--------------

Before a run:

* generate a linear matter power spectrum with consistent cosmology, redshift,
  normalization, and units;
* prepare a two-column weak-lensing kernel when ``Wg=1``;
* choose a unique ``rootDir`` and ``prefix``;
* begin with reduced numerical controls and establish convergence;
* archive the exact input tables and configuration.

Post-Processing
---------------

Read the angular grid from ``<rootDir>/<prefix>theta_array.txt`` and the 3PCF
multipoles from ``<rootDir>/<prefix>zetam<m>.txt``.  Related bispectrum and
background products use the same directory and prefix.

Typical tasks include plotting multipoles, checking matrix shapes and finite
values, comparing model branches, measuring numerical convergence, and
assembling training vectors for the optional emulator.  Always retain the
``*-usedvalues`` file and ``<prefix>info.txt`` with derived products.

See :doc:`io_formats` for exact names and :doc:`tutorials/python-3pcf` for a
minimal plotting example.
