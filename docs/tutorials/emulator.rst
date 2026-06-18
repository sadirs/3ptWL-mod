Neural-Network Emulator Workflow
================================

The emulator workflow accelerates repeated 3ptWL-mod evaluations after a
direct training set has been generated.  It is useful for parameter scans and
inference demonstrations, but every application should validate emulator
predictions against direct, converged model runs.

Workflow Components
-------------------

``tests/emulator.py``
   Shared implementation for cosmology-grid generation, CAMB power spectra,
   direct 3ptWL-mod runs, vector preparation, neural-network training, weight
   serialization, and diagnostics.

``tests/emulator.ipynb``
   Generates the training design and trains the surrogate models.

``tests/use_wlcf_emulator.ipynb``
   Loads trained weights, predicts a test cosmology, and demonstrates a compact
   ``emcee`` inference problem with an artificial covariance.

``tests/firecrown_emulator_likelihood.ipynb``
   Experimental scaffold for exposing the emulator through a Gaussian
   likelihood and optional Firecrown interfaces.

Current Training Design
-----------------------

The default helper configuration uses a 300-point Latin-hypercube design over
``Omega_m``, ``h``, and ``logAs`` at ``z=0.5078`` and models multipoles zero
through eight.  Generated power spectra, direct 3PCF vectors, trained weights,
runtime logs, and diagnostic products live under ignored directories in
``tests/`` so they can be regenerated rather than committed.

Run the Workflow
----------------

Create an environment containing CAMB, SciPy, pandas, scikit-learn,
Matplotlib, Jupyter, ``emcee``, and ``corner`` in addition to the compiled
``wlcfpy`` extension.  Then run the notebooks in order:

.. code-block:: bash

   cd tests
   jupyter lab emulator.ipynb
   jupyter lab use_wlcf_emulator.ipynb
   jupyter lab firecrown_emulator_likelihood.ipynb

The helper sends all model outputs through ``rootDir`` and uses ``prefix`` to
separate cosmology samples.  Interrupted grids can be resumed from existing
vectors and runtime logs.

Validation Checklist
--------------------

* reserve validation and test cosmologies that were not used for training;
* inspect absolute and relative errors for every retained multipole;
* compare representative predictions with direct 3ptWL-mod runs;
* do not extrapolate beyond the documented training bounds;
* propagate an emulator-error model in any scientific likelihood;
* replace the demonstration covariance with a validated analysis covariance.

Neural-Network Acceleration Reference
-------------------------------------

The broader strategy of accelerating configuration-space cosmology with neural
networks is demonstrated in:

  Sadi Ramirez, Miguel Icaza-Lizaola, Sebastien Fromenteau, Mariana
  Vargas-Magaña, and Alejandro Aviles, *Full shape cosmology analysis from BOSS
  in configuration space using neural network acceleration*, Journal of
  Cosmology and Astroparticle Physics **2024** (08) 049 (2024),
  `doi:10.1088/1475-7516/2024/08/049`_.

That paper provides methodological motivation for the acceleration strategy;
the 3ptWL-mod notebooks are a separate weak-lensing 3PCF implementation and
must be validated on their own training domain.

.. _doi:10.1088/1475-7516/2024/08/049: https://doi.org/10.1088/1475-7516/2024/08/049
