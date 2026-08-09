3ptWL-mod: Weak-Lensing Three-Point Modeling
--------------------------------------------

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

Authors
~~~~~~~

* Alejandro Aviles (ICF-UNAM, Mexico), avilescervantes@gmail.com, aviles@icf.unam.mx
* Juan Carlos Hidalgo (ICF-UNAM, Mexico)
* Eladio Moreno (UGTO, Mexico)
* Gustavo Niz (UGTO, Mexico)
* Mario A. Rodriguez-Meza (ININ, Mexico)
* Sofía Samario (ICF-UNAM, Mexico), ssamario@icf.unam.mx
* Sadi Ramírez (ICF-UNAM, Mexico), sadi@icf.unam.mx

Repository: `Source code and issue tracking`_

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

Acknowledgements
~~~~~~~~~~~~~~~~

We acknowledge financial support from grants DGAPA-PAPIIT IA101825 and
SECIHITI CBF2023-2024-162.

.. _Source code and issue tracking: https://github.com/sadirs/3ptWL-mod
.. _3ptWL-cov: https://3ptwl-cov.readthedocs.io/en/latest/overview.html
.. _cTreeBalls: https://github.com/rodriguezmeza/cTreeBalls
.. _arXiv:2408.16847: https://arxiv.org/abs/2408.16847
.. _Read the Docs: https://3ptwl-mod.readthedocs.io/en/latest/
