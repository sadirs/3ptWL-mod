Pre/Post Processing
===================

This section summarizes the work usually done before and after a ``wlcf`` run.

Pre-Processing
--------------

Prepare all inputs before running the executable or Python wrapper:

* Generate a linear matter power spectrum with CAMB, CLASS, or another
  trusted source.
* Save the power spectrum as a plain numeric two-column file accepted by
  ``fnamePS``/``ps``.
* If ``Wg=1``, prepare a plain numeric two-column weak-lensing kernel file for
  ``fWgchi``.
* Use cosmological parameters consistent with the input tables.
* Keep relative paths consistent with the working directory from which
  ``wlcf`` is launched.

The accuracy of the final correlation functions depends strongly on the
quality, units, redshift, and cosmology of the input power spectrum and kernel.

Post-Processing
---------------

Post-processing usually starts from files in ``path_Bells``:

* ``zetam*.txt`` for 3PCF multipoles.
* ``theta_array.txt`` for the angular grid.
* ``Bmells_*.txt`` for projected bispectrum multipoles.
* ``Bnk_*.txt`` and ``kArray.txt`` for Fourier-space bispectrum tables.
* ``background_functions.txt`` for sampled background quantities.
* ``info.txt`` for a compact run summary.

Typical post-processing tasks include plotting multipoles, comparing model
branches, checking convergence with ``Nell`` or ``chiQuadSteps``, and combining
outputs from parameter scans.

For exact file naming and column expectations, see :doc:`io_formats`.
