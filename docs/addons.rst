Optional Addons and Extension Points
====================================

The ``addons/`` tree contains parser support, generated-declaration fragments,
dependency notes, and include sockets used to assemble the public build.

Standard Build Addons
---------------------

``addons/class_lib``
   CLASS-style parameter-file parser and associated error handling.

``addons/pxd``
   Cython declaration fragments used to generate ``python/cwlcfpy.pxd``.

``addons/addons_include``
   Source and structure fragments inserted at marked addon sockets.

``addons/gsl`` and ``addons/fftw3``
   Dependency notes and optional bundled material.  System or environment
   packages are preferred for normal builds.

Adding a Parameter
------------------

A new run-time parameter requires coordinated changes to the defaults,
``cmdline_data`` structure, command-line reader, CLASS-style parser, reporting
logic, cleanup logic, and Cython template when exposed to Python.  Rebuild from
clean objects and confirm the wrapper ABI check.

Adding Numerical Code
---------------------

Place public declarations in ``include/`` and implementations in ``source/``.
Add the object to the relevant Makefile list and expose it through a stable
entry point rather than importing internal globals into Python.  New model
branches should document their assumptions, parameters, output products, and
convergence tests.

See :doc:`development` for the validation checklist.
