3PCF Model Reference
====================

3ptWL-mod models the three-point correlation function of projected scalar
fields on the sphere.  Its primary application is weak-lensing convergence,
but the harmonic construction applies more broadly to spin-zero projected
fields.

Harmonic Representation
-----------------------

For angular separations :math:`\theta_1` and :math:`\theta_2`, the code
represents the azimuthal dependence through multipoles

.. math::

   \zeta_m(\theta_1,\theta_2), \qquad m=0,\ldots,m_{\max}.

The decomposition turns the angular structure into a sequence of two-dimensional
matrices.  The numerical pipeline computes matter-bispectrum quantities,
projects them through the lensing kernel under the adopted approximations, and
transforms the projected multipoles into configuration space.

Model Branches
--------------

``tree_level=1``
   Standard perturbation-theory branch.

``tree_level=2``
   Power-spectrum-squared approximation.

``tree_level=3``
   Effective-field-theory inspired branch.

``tree_level=4``
   Takahashi/Halo-model inspired branch used by the main notebook examples.

The branches are different physical approximations, not successive numerical
accuracy levels.  Compare them only with consistent cosmology, inputs, grids,
and source kernels.

Projection Inputs
-----------------

The calculation consumes a linear matter power spectrum through ``fnamePS``.
``Wg=0`` uses a source plane at ``zbin``; ``Wg=1`` reads the tabulated kernel
selected by ``fWgchi``.  See :doc:`ps_files` for formats and units.

Numerical Grids
---------------

``Nell``, ``ellmin``, and ``ellmax`` define the multipole grid.
``chiQuadSteps`` and ``GLpoints`` control radial and angular integrations.
``mMax`` selects the highest output moment.  Convergence must be established
for the angular range and model branch used in the analysis.

Outputs
-------

The angular grid is written to ``<rootDir>/<prefix>theta_array.txt`` and each
moment to ``<rootDir>/<prefix>zetam<m>.txt``.  Bispectrum, background, and grid
products are documented in :doc:`io_formats`.

Scientific Reference
--------------------

The modeling framework is described in `Modeling the 3-point correlation
function of projected scalar fields on the sphere`_.  Record the code commit,
inputs, numerical settings, and model branch whenever publishing derived
results.

.. _Modeling the 3-point correlation function of projected scalar fields on the sphere: https://arxiv.org/abs/2408.16847
