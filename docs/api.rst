API Reference
=============

Command-Line Interface
----------------------

``wlcf`` accepts ``key=value`` arguments and ``paramfile=<path>``.  Run
``./wlcf --help`` for the compiled interface.  The executable writes all
products below ``rootDir`` and returns a nonzero status on initialization or
computation failure.

Python Module
-------------

The public extension exports the ``wlcf`` class from ``wlcfpy``.

``wlcf(default=False)``
   Construct an uninitialized model wrapper.  The current constructor also
   accepts ``default=True`` to populate a minimal default parameter set.

``set(*args, **kwargs)``
   Merge one parameter dictionary and/or keyword arguments into the pending
   configuration.

``Run(level=["EndRun"])``
   Validate parameters, initialize C state, execute the requested stages, and
   return a CPU-time value.

``clean()`` and ``clean_all()``
   Reset parameter/computation state and release allocated C resources.

``abi_sizes()``
   Report the C structure sizes used by the extension's ABI validation.

C Implementation Map
--------------------

``main/main.c``
   Command-line entry point.

``source/startrun.c``
   Parameter ingestion, validation, directories, logging, and allocations.

``source/wlcf.c`` and ``source/wlcfio.c``
   Public run stages and lifecycle/cleanup operations.

``source/background.c``
   Background and lensing-kernel quantities.

``source/procedures.c`` and ``source/zetam.c``
   Bispectrum projection and 3PCF production.

``source/twobessel.c``
   FFTW-based two-Bessel transform routines.

``python/wlcfpy.pyx``
   Python-facing Cython implementation.

The internal C interfaces are not yet versioned as a stable external library
API.  Pin a repository commit when linking custom code against ``libwlcf.a``.
