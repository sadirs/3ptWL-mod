3ptWL-mod
=========

**3ptWL-mod** computes theoretical multipoles of the three-point correlation
function (3PCF) of projected scalar fields, with a focus on weak-lensing
convergence.  The public workflow provides:

* a compiled C command-line executable, ``wlcf``;
* a static library, ``libwlcf.a``;
* a Cython wrapper, ``wlcfpy``;
* a standalone Google Colab comparison of all four model branches;
* notebook workflows for 3PCF visualization and neural-network emulation.

The models include perturbation-theory, effective-field-theory, and
Takahashi/Halo-model inspired bispectrum branches.  Although the repository is
named 3ptWL-mod, the executable and Python module retain their historical
``wlcf`` names for compatibility.

Start in Google Colab
---------------------

The `standalone four-model notebook`_ installs the published package, fetches
a versioned example input, and compares SPT, Tree, EFT, and Halo Model without
requiring a repository clone.  Its shared-scale multipole maps provide the
quickest visual introduction to the model branches.  See
:doc:`tutorials/colab-four-models` for the workflow, settings, generated files,
and convergence guidance.

Basic Usage
-----------

Build the executable, static library, and Python wrapper from a source checkout:

.. code-block:: bash

   make clean
   make PYTHON=python3 all

Run a compact command-line calculation:

.. code-block:: bash

   ./wlcf rootDir=Output_quick prefix=quick_ \
      fnamePS=./input/linear_pk_Takahashi_z0.txt \
      numberThreads=1 verbose=0 verbose_log=0 \
      mMax=2 Nell=32 chiQuadSteps=40 GLpoints=24 writevectors=false

Or use the Python wrapper:

.. code-block:: python

   from wlcfpy import wlcf

   model = wlcf()
   model.set({
       "rootDir": "Output_python",
       "prefix": "python_",
       "fnamePS": "./input/linear_pk_Takahashi_z0.txt",
       "tree_level": 4,
       "mMax": 2,
       "Nell": 32,
       "chiQuadSteps": 40,
       "GLpoints": 24,
       "numberThreads": 1,
       "verbose": 0,
       "verbose_log": 0,
       "writevectors": False,
   })
   cputime = model.Run()
   model.clean_all()

How to Use This Guide
---------------------

Start with :doc:`installation` and :doc:`quickstart`.  For production runs,
read :doc:`params`, :doc:`ps_files`, and :doc:`io_formats` before increasing
the numerical resolution.  The :doc:`tutorials/index` section begins with the
standalone Colab comparison and continues through the end-to-end 3PCF and
emulator workflows.  Developers should also consult :doc:`development` and
:doc:`troubleshooting`.

.. toctree::
   :maxdepth: 2
   :caption: User Guide

   overview
   installation
   quickstart
   params
   configuration
   ps_files
   io_formats
   python
   performance

.. toctree::
   :maxdepth: 2
   :caption: Tutorials

   tutorials/index

.. toctree::
   :maxdepth: 2
   :caption: Reference

   3pcf
   addons
   api
   troubleshooting
   development
   citing

.. _standalone four-model notebook: https://colab.research.google.com/github/sadirs/3ptWL-mod/blob/main/examples/3ptWL_mod_four_models_colab.ipynb
