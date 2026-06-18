Build Configuration
===================

The build is controlled by ``Makefile_settings``, ``Makefile_machine``, and
``addons/Makefile_addons_settings``.

Makefile Settings
-----------------

``USEGSL``
   Enables GSL-dependent numerical routines. The public configuration expects
   this to remain enabled.

``OPENMPMACHINE``
   Enables OpenMP compilation and the ``numberThreads`` run-time parameter.

``ADDONSON``
   Enables the CLASS-style parser and Cython support sockets required by the
   public Python wrapper.

Addon Settings
--------------

The standard wrapper build uses ``CLASSLIBON=1`` and ``PXDON=1``.  The PXD
templates are expanded during the Python build so Cython sees the same
structures and compile-time options as the C library.

Compiler and Linker Variables
-----------------------------

``CC`` and ``AR`` select the compiler and archive tool. ``PYTHON`` selects the
interpreter used for the extension build.  Dependency paths can be supplied
with ``GSL_INCLUDE``, ``GSL_LIB``, ``FFTW3_INCLUDE``, and ``FFTW3_LIB``.

Recommended Pattern
-------------------

Make configuration changes before a clean rebuild:

.. code-block:: bash

   make clean
   make PYTHON=python3 all

Do not reuse object files after changing OpenMP, addon, compiler, or ABI-related
settings.  The wrapper performs an ABI-size check during initialization and
will reject a mismatched extension/library pair.

Build Artifacts
---------------

The build produces ``wlcf``, ``libwlcf.a``, object files beneath ``build/``, a
generated ``python/cwlcfpy.pxd``, and an installed ``wlcfpy`` extension.  The
generated products are excluded from version control.
