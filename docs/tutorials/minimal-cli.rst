Minimal Command-Line Workflow
=============================

This tutorial builds the code, runs a reduced Halo-model calculation, and
checks the output layout.

Build
-----

.. code-block:: bash

   git clone https://github.com/sadirs/3ptWL-mod.git
   cd 3ptWL-mod
   make clean
   make PYTHON=python3 all

Run
---

.. code-block:: bash

   ./wlcf rootDir=Output_tutorial prefix=tutorial_ \
      fnamePS=./input/linear_pk_Takahashi_z0.txt \
      tree_level=4 zbin=0.5078 \
      mMax=2 Nell=32 chiQuadSteps=40 GLpoints=24 \
      numberThreads=1 verbose=1 verbose_log=0 writevectors=false

Validate
--------

.. code-block:: bash

   test -s Output_tutorial/tutorial_theta_array.txt
   test -s Output_tutorial/tutorial_zetam0.txt
   test -s Output_tutorial/tutorial_info.txt

Open the used-parameter file and verify that its values match the command.  For
a production calculation, increase the numerical controls gradually and follow
the convergence procedure in :doc:`../performance`.

Reuse a Parameter File
----------------------

Copy the generated ``*-usedvalues`` file, edit the desired values, and run:

.. code-block:: bash

   ./wlcf paramfile=my_tutorial_run.ini

This keeps the full configuration explicit and reproducible.
