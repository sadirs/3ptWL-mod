Python and 3PCF Tutorial
========================

The version-controlled ``tests/example.ipynb`` notebook implements this
workflow interactively.  The outline below shows the same public interface in a
compact script.

Prepare an Input Spectrum
-------------------------

Use the bundled table for a first run:

.. code-block:: python

   from pathlib import Path

   pk_file = Path("input/linear_pk_Takahashi_z0.txt")
   if not pk_file.exists():
       raise FileNotFoundError(pk_file)

For a scientific analysis, generate a power spectrum with CAMB, CLASS, or a
validated emulator using the same cosmology, redshift, and units as the run.

Run 3ptWL-mod
-------------

.. code-block:: python

   from wlcfpy import wlcf

   output = Path("Output_python_tutorial")
   model = wlcf()
   model.set({
       "rootDir": str(output),
       "prefix": "halo_",
       "fnamePS": str(pk_file),
       "tree_level": 4,
       "zbin": 0.5078,
       "mMax": 4,
       "Nell": 64,
       "chiQuadSteps": 120,
       "GLpoints": 32,
       "numberThreads": 4,
       "verbose": 1,
       "verbose_log": 0,
       "writevectors": False,
   })
   model.Run()
   model.clean_all()

Plot Multipoles
---------------

.. code-block:: python

   import matplotlib.pyplot as plt
   import numpy as np

   theta = np.loadtxt(output / "halo_theta_array.txt")
   theta_arcmin = np.degrees(theta) * 60.0
   zeta0 = np.loadtxt(output / "halo_zetam0.txt")

   plt.pcolormesh(theta_arcmin, theta_arcmin, zeta0, shading="auto")
   plt.xscale("log")
   plt.yscale("log")
   plt.xlabel(r"$\theta_2$ [arcmin]")
   plt.ylabel(r"$\theta_1$ [arcmin]")
   plt.colorbar(label=r"$\zeta_0$")
   plt.show()

Compare Models
--------------

Repeat the run with a different ``tree_level`` and a distinct ``rootDir``.
Compare the same multipole and angular region, and do not mix input spectra or
cosmological parameters between branches.  See :doc:`../3pcf` for the model
interpretation.
