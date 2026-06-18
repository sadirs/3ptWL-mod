Troubleshooting and Common Errors
=================================

Undefined FFTW Symbols
----------------------

Errors mentioning ``fftw_malloc`` or ``fftw_execute`` mean the final link did
not include FFTW3.  Install the development package or provide
``FFTW3_INCLUDE`` and ``FFTW3_LIB`` explicitly, then run ``make clean``.

Undefined GSL Symbols
---------------------

Confirm that ``gsl-config --cflags`` and ``gsl-config --libs`` work, or provide
``GSL_INCLUDE`` and ``GSL_LIB``.  Do not reuse objects compiled with a different
GSL configuration.

``python: command not found``
-----------------------------

Select Python 3 explicitly:

.. code-block:: bash

   make PYTHON=python3 all

Missing ``libwlcf.a``
---------------------

The extension links against the static library.  Build it first with
``make libwlcf.a`` or use ``make all``.

ABI Mismatch During Wrapper Initialization
------------------------------------------

The generated PXD declarations, compile-time addon settings, and static
library disagree.  Remove generated artifacts with ``make clean`` and rebuild
the complete stack with one compiler and configuration.

Unread or Unknown Parameters
-----------------------------

Compare names with ``./wlcf --help``.  The current output directory is
``rootDir`` and the current verbosity controls are ``verbose`` and
``verbose_log``.  Older ``path_Bells`` and ``chatty`` parameters are not part of
the public interface.

Missing Input Files
-------------------

Relative paths are resolved from the process working directory.  Print
``Path.cwd()`` in Python or use absolute paths while diagnosing a notebook.

NaN or Unstable 3PCF Products
-----------------------------

Check the power-spectrum units and redshift, lensing kernel, angular range, and
model branch.  Repeat with increased ``Nell``, ``chiQuadSteps``, and
``GLpoints`` one at a time.  A surrogate prediction should never replace this
direct-model convergence check.

Build Warnings
--------------

The upstream C sources currently emit some conservative ``snprintf``
truncation warnings under fortified GCC builds.  Treat new warnings as review
items and keep warnings distinct from successful validation results.
