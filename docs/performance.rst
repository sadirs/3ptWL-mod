Performance and Parallelization
===============================

Integration Controls
--------------------

The main cost drivers are ``Nell``, ``chiQuadSteps``, ``GLpoints``, ``mMax``,
and the selected model branch.  Larger values are not automatically better:
increase one control at a time and compare the final science products.

OpenMP
------

Compile with ``OPENMPMACHINE=1`` and select the run-time thread count with
``numberThreads`` or ``OMP_NUM_THREADS``:

.. code-block:: bash

   export OMP_NUM_THREADS=8
   ./wlcf rootDir=Output_parallel numberThreads=8

Do not request more threads than the scheduler allocation on a shared system.
For small validation runs, thread startup can dominate the elapsed time.

Parameter Scans
---------------

Independent cosmologies or model branches are embarrassingly parallel.  Use a
separate ``rootDir`` for each process and avoid oversubscribing cores by
combining too many processes with too many OpenMP threads.

Convergence Checks
------------------

For every production configuration:

1. establish a reduced baseline;
2. increase ``Nell`` and compare ``zetam*``;
3. increase ``chiQuadSteps`` and ``GLpoints`` independently;
4. verify the requested multipoles and angular range;
5. compare the direct model with any emulator prediction used downstream.

Record both numerical controls and software versions with the results.
