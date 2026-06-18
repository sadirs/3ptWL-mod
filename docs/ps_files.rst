Inputs and Parameter Files
==========================

Linear Power Spectrum
---------------------

``fnamePS`` (alias ``ps``) selects a header-free numeric ASCII table with two
columns:

* wavenumber :math:`k` in ``h/Mpc``;
* linear matter power spectrum :math:`P(k)` in ``(Mpc/h)^3``.

Example:

.. code-block:: text

   1.000000e-03  2.345678e+04
   2.000000e-03  1.987654e+04
   5.000000e-03  1.234567e+04

Generate the table with CAMB, CLASS, or another validated source.  Its
redshift, normalization, cosmology, and units must agree with the run
parameters.

Weak-Lensing Kernel
-------------------

When ``Wg=1``, ``fWgchi`` selects another header-free, two-column table:

* comoving distance ``chi`` in ``Mpc/h``;
* lensing-kernel value ``Wg(chi)``.

With ``Wg=0``, the code instead uses the source plane defined by ``zbin``.

Parameter Files
---------------

A parameter file contains the same names shown by ``./wlcf --help``.  Start
from a generated ``*-usedvalues`` file whenever possible.  Relative input paths
are resolved from the process working directory, not from the parameter-file
location.

Reproducibility
---------------

Archive the input tables, used-parameter file, repository commit, compiler and
dependency versions, and output ``info.txt`` together.  A power-spectrum file
with the correct shape but inconsistent cosmology can produce a plausible yet
scientifically invalid result.
