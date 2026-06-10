Linear Power Spectrum Files
===========================

``wlcf`` reads the linear matter power spectrum from the file selected by
``fnamePS`` or its alias ``ps``.

Format
------

The file must be a plain numeric ASCII table with two columns:

* ``k`` in units of ``h/Mpc``.
* ``P(k)`` in units of ``(Mpc/h)^3``.

Example:

.. code-block:: text

    1.000000e-03  2.345678e+04
    2.000000e-03  1.987654e+04
    5.000000e-03  1.234567e+04

Do not include header or comment lines in this file. The current reader expects
two floating-point values per row.

Usage
-----

From ``tests``:

.. code-block:: bash

    ../wlcf ps=./input/linear_pk_Takahashi_z0.txt

Equivalent long-form parameter:

.. code-block:: bash

    ../wlcf fnamePS=./input/linear_pk_Takahashi_z0.txt

The power spectrum can be generated with Boltzmann solvers such as CAMB or
CLASS. Make sure the redshift, cosmology, and units are consistent with the
rest of the run configuration.

See Also
--------

See :doc:`io_formats` for the weak-lensing kernel and output table formats.
