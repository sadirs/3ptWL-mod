Input And Output File Formats
=============================

This page summarizes the plain-text files that ``wlcf`` reads and writes in
the standard 3PCF workflow.

Input: Linear Power Spectrum
----------------------------

The file selected by ``fnamePS`` or its alias ``ps`` is read as two numeric
columns:

.. code-block:: text

    1.000000e-03  2.345678e+04
    2.000000e-03  1.987654e+04
    5.000000e-03  1.234567e+04

Column meanings:

* Column 1: wavenumber ``k`` in ``h/Mpc``.
* Column 2: linear matter power spectrum ``P(k)`` in ``(Mpc/h)^3``.

Keep the file numeric. The current C reader expects two floating-point values
per row and does not skip header lines.

Input: Weak-Lensing Kernel
--------------------------

When ``Wg=1``, the file selected by ``fWgchi`` is read as two numeric columns:

.. code-block:: text

    1.000000e+01  0.000000e+00
    2.000000e+01  1.251337e-04
    3.000000e+01  2.488925e-04

Column meanings:

* Column 1: comoving distance ``chi`` in ``Mpc/h``.
* Column 2: weak-lensing kernel value ``Wg(chi)``.

As with the power spectrum, keep this file numeric and header-free.

Output Directories
------------------

``rootDir`` stores run-level outputs such as logs and used-parameter files.
The default is ``Output``.

``path_Bells`` stores the angular 3PCF and bispectrum tables. The default is
``Bell_outputs``.

Most files written to ``path_Bells`` use the configured ``prefix``. With the
default prefix ``run1_``, files are named like ``run1_zetam0.txt`` and
``run1_Bmells_0.txt``.

Run Metadata
------------

``<rootDir>/*-usedvalues``
    Parameter values used in the run. Use this as the safest starting point
    for a new parameter file.

``<rootDir>/tmp/wlcf.log``
    Log output when ``verbose_log`` is greater than zero.

``<path_Bells>/<prefix>info.txt``
    Summary of cosmology and numerical grid settings written by the run.

Background Tables
-----------------

``<path_Bells>/<prefix>background_functions.txt``
    Table with redshift, comoving distance, growth factor, nonlinear scale,
    effective spectral index, and lensing-weight quantities sampled on the
    radial integration grid.

Grid Tables
-----------

``<path_Bells>/<prefix>theta_array.txt``
    Angular separation grid used by ``zetam*.txt``.

``<path_Bells>/<prefix>ellArray.txt``
    Multipole grid used by ``Bmells_*.txt``.

``<path_Bells>/<prefix>kArray.txt``
    Wavenumber grid used by ``Bnk_*.txt``.

3PCF And Bispectrum Tables
--------------------------

``<path_Bells>/<prefix>zetam<m>.txt``
    Matrix of the 3PCF multipole ``zeta_m(theta1, theta2)`` for each
    multipole ``m`` from ``0`` through ``mMax``.

``<path_Bells>/<prefix>Bmells_<m>.txt``
    Matrix of projected bispectrum multipole values on the ``ellArray`` grid.

``<path_Bells>/<prefix>BmellsVector_<m>.txt``
    Flattened version of the projected bispectrum matrix, written when
    ``writevectors`` is enabled.

``<path_Bells>/<prefix>Bnk_<m>.txt``
    Matrix of Fourier-space bispectrum values on the ``kArray`` grid.
