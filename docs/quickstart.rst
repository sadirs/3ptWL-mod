Quickstart
==========

This page runs a deliberately reduced calculation so a new installation can be
validated before starting a production 3PCF run.

Build
-----

From the repository root:

.. code-block:: bash

   make clean
   make PYTHON=python3 all

Run the CLI
-----------

.. code-block:: bash

   ./wlcf rootDir=Output_quick prefix=quick_ \
      fnamePS=./input/linear_pk_Takahashi_z0.txt \
      tree_level=4 numberThreads=1 \
      mMax=2 Nell=32 chiQuadSteps=40 GLpoints=24 \
      verbose=0 verbose_log=0 writevectors=false

The code creates ``Output_quick`` and writes the angular grid, model tables,
3PCF multipoles, run metadata, and used-parameter file beneath it.

Inspect the Products
--------------------

.. code-block:: bash

   ls Output_quick

The most useful first checks are:

* ``quick_theta_array.txt`` for the angular grid;
* ``quick_zetam0.txt`` through ``quick_zetam2.txt`` for 3PCF multipoles;
* ``quick_info.txt`` for run metadata;
* ``parameters_null-wlcf-usedvalues`` for the complete parameter state.

Run the Python Wrapper
----------------------

.. code-block:: python

   from pathlib import Path
   from wlcfpy import wlcf

   output = Path("Output_python")
   model = wlcf()
   model.set({
       "rootDir": str(output),
       "prefix": "python_",
       "fnamePS": "./input/linear_pk_Takahashi_z0.txt",
       "tree_level": 4,
       "numberThreads": 1,
       "mMax": 2,
       "Nell": 32,
       "chiQuadSteps": 40,
       "GLpoints": 24,
       "verbose": 0,
       "verbose_log": 0,
       "writevectors": False,
   })
   print(model.Run())
   model.clean_all()

Next Steps
----------

Read :doc:`params` before changing the model branch or integration controls.
Use :doc:`performance` to design convergence checks, then continue with the
:doc:`tutorials/index` workflows.
