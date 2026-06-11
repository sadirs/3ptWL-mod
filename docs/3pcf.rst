
3-point correlation functions
=============================

**3ptWL-mod** computes **three-point correlation functions (3PCF)** for scalar fields, such as the weak lensing convergence :math:`\kappa`. These higher-order statistics provide access to non-Gaussian information in the large-scale structure of the Universe.

In particular, the code evaluates the correlation:

.. math::
    
    \langle \kappa(\hat{n}_1),\kappa(\hat{n}_2),\kappa(\hat{n}_3) \rangle

using a plane-wave expansion and a harmonic decomposition.

**Harmonic base**

The angular dependence of the 3PCF is expressed in a **harmonic basis**, where the signal is decomposed into multipoles. This allows for an efficient and structured representation of the correlation function.

The expansion is performed in terms of angular modes labeled by an index :math:`m`, leading to a set of coefficients:

.. math::
    
    \zeta_m(\theta_1, \theta_2)

where:

* :math:`\theta_1`, :math:`\theta_2` are angular separations
* :math:`m = 0, 1, \dots, m_{\mathrm{max}}` is the multipole index


**Output**

The results of the harmonic decomposition are stored in files under
``path_Bells``. File names use the configured ``prefix``:

::

    <prefix>zetam0.txt
    <prefix>zetam1.txt
    ...
    <prefix>zetamN.txt

along with the corresponding angular grid:

::
    
    <prefix>theta_array.txt


These outputs can be used to reconstruct or visualize the full three-point correlation function.

See :doc:`io_formats` for the related ``Bmells_*``, ``Bnk_*``, and grid files.
