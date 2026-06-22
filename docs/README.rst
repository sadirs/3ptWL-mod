3ptWL-mod: Weak-Lensing Three-Point Modeling
--------------------------------------------

Project Team
~~~~~~~~~~~~

:Scientific authors: Alejandro Aviles, Juan Carlos Hidalgo, Eladio Moreno,
   Gustavo Niz, Mario A. Rodriguez-Meza, Sofía Samario, and collaborators.
:Emulator workflow: Sadi Ramirez and contributors.
:Repository: `Source code and issue tracking`_

Scientific Scope
~~~~~~~~~~~~~~~~

3ptWL-mod is a C code for modeling multipoles of the three-point correlation
function of projected scalar fields on the sphere.  Its main application is
the weak-lensing convergence field.  The numerical pipeline projects a matter
bispectrum into angular multipoles and evaluates
:math:`\zeta_m(\theta_1,\theta_2)` on a configurable angular grid.

The available model branches include standard perturbation theory, a
power-spectrum-squared approximation, an effective-field-theory prescription,
and a Takahashi/Halo-model inspired branch.  See :doc:`3pcf` for the scientific
conventions and :doc:`params` for the run-time controls.

Public Interfaces
~~~~~~~~~~~~~~~~~

The repository provides the ``wlcf`` command-line executable,
``libwlcf.a`` static library, and ``wlcfpy`` Python extension.  These names are
retained from the original ``wlcf`` project so existing analysis scripts can
continue to use the public API while the repository name describes its
scientific purpose more directly.

All run products are written beneath ``rootDir`` and use ``prefix`` where
applicable.  Typical products include the angular grid, 3PCF multipoles,
bispectrum multipoles, background tables, and a complete used-parameter file.

Related Projects
~~~~~~~~~~~~~~~~

* `3ptWL-cov`_ computes Gaussian weak-lensing three-point covariance terms.
* `cTreeBalls`_: measures two- and three-point correlation functions from point
  catalogs and scalar fields.
* The model implemented here accompanies the projected-scalar-field 3PCF work
  described in `arXiv:2408.16847`_.

Installing and Getting Started
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Read :doc:`installation` for dependencies and build configuration, then follow
:doc:`quickstart` for a reduced validation run.  For a no-clone visual start,
open the :doc:`tutorials/colab-four-models` notebook.  The
:doc:`tutorials/index` section covers the full Python and neural-network
workflows.

Documentation Builds
~~~~~~~~~~~~~~~~~~~~

Build the HTML documentation with:

.. code-block:: bash

   python3 -m pip install -r docs/requirements.txt
   make -C docs html

The hosted documentation is available on `Read the Docs`_.

License
~~~~~~~

3ptWL-mod is distributed under the MIT license.  See :doc:`citing` for the
scientific references and acknowledgement guidance.

.. _Source code and issue tracking: https://github.com/sadirs/3ptWL-mod
.. _3ptWL-cov: https://3ptwl-cov.readthedocs.io/en/latest/overview.html
.. _cTreeBalls: https://github.com/rodriguezmeza/cTreeBalls
.. _arXiv:2408.16847: https://arxiv.org/abs/2408.16847
.. _Read the Docs: https://3ptwl-mod.readthedocs.io/en/latest/
