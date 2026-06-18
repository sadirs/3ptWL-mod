Python Wrapper
==============

3ptWL-mod can be driven through ``subprocess`` or through the compiled
``wlcfpy`` extension.

Calling the Executable
----------------------

.. code-block:: python

   import subprocess

   subprocess.run(
       [
           "./wlcf",
           "rootDir=Output_subprocess",
           "prefix=subprocess_",
           "tree_level=4",
           "fnamePS=./input/linear_pk_Takahashi_z0.txt",
           "numberThreads=4",
       ],
       check=True,
   )

Each parameter must be one ``key=value`` argument.

Using ``wlcfpy``
----------------

Build and import the extension:

.. code-block:: bash

   make PYTHON=python3 all
   python3 -c "from wlcfpy import wlcf; print(wlcf)"

A minimal run is:

.. code-block:: python

   from wlcfpy import wlcf

   model = wlcf()
   model.set({
       "rootDir": "Output_python",
       "prefix": "python_",
       "fnamePS": "./input/linear_pk_Takahashi_z0.txt",
       "tree_level": 4,
       "numberThreads": 4,
       "verbose": 1,
       "verbose_log": 0,
   })
   cputime = model.Run()
   print(cputime)
   model.clean_all()

Wrapper Semantics
-----------------

``set`` accepts keyword arguments or one dictionary. ``Run`` validates the
parameters, initializes the numerical structures, executes the requested
stages, and returns a CPU-time value. ``clean_all`` releases the allocated C
state.  Create separate wrapper instances for independent concurrent jobs.

The extension checks the C and Cython structure sizes during initialization.
An ABI mismatch means the static library, generated PXD declarations, and
extension were built with inconsistent settings; run a clean rebuild.

Relative Paths
--------------

Input paths are resolved relative to the Python process working directory.
Every output follows the same ``rootDir`` and ``prefix`` rules as the CLI.

See :doc:`tutorials/python-3pcf` for a complete notebook-oriented workflow and
:doc:`tutorials/emulator` for the optional surrogate-model pipeline.
