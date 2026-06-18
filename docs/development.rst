Developer Guide
===============

Repository Layout
-----------------

``source/``, ``include/``, ``main/``
   Numerical implementation, public structures, and executable entry point.

``getparam/`` and ``addons/class_lib/``
   Command-line and parameter-file readers.

``general_lib/``
   Shared C utilities.

``python/`` and ``addons/pxd/``
   Cython wrapper and generated declaration fragments.

``tests/``
   Version-controlled notebooks, emulator helper, and input tables.

``docs/``
   Sphinx and manual-page sources.

Validation Checklist
--------------------

Before submitting a change:

.. code-block:: bash

   make clean
   make PYTHON=python3 all
   ./wlcf --help
   python3 -c "from wlcfpy import wlcf; print(wlcf().abi_sizes())"
   sphinx-build -W -b html docs docs/_build/html

Also run a reduced direct calculation, verify expected output files, and check
that no generated products are staged.

Changing Parameters
-------------------

Keep defaults, C structures, both parser paths, reporting, cleanup, and Cython
declarations synchronized.  Remove obsolete names rather than leaving defaults
that no parser consumes.  A new output location should follow the single
``rootDir`` convention unless there is a strong, documented reason not to.

Documentation Style
-------------------

Use descriptive page titles, runnable commands, repository-relative paths, and
explicit caveats.  Organize pages under User Guide, Tutorials, or Reference.
Build with warnings treated as errors because Read the Docs uses
``fail_on_warning: true``.

Emulator Changes
----------------

Keep ``tests/emulator.py`` as the shared source of truth for notebook logic.
Document parameter bounds, training design, data splits, error metrics, and
covariance assumptions.  Do not commit generated grids or weights unless a
versioned release process is established.
