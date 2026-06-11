3ptWL-mod: Weak-Lensing Three-Point Modeling
--------------------------------------------

3ptWL-mod is a C code for computing weak-lensing correlation-function models.
The current workflow focuses on the three-point correlation function (3PCF) of
the weak-lensing convergence field using perturbation-theory, EFT, and
Takahashi/Halo-model inspired branches.

The project was previously published as ``wlcf``.  The executable, static
library, Python extension, environment variables, and public APIs retain their
existing compatibility names.

For source code and releases, see:

https://github.com/sadirs/3ptWL-mod

The associated paper is available at:

https://arxiv.org/abs/2408.16847

Quick Start
-----------

Clone and build:

.. code-block:: bash

    git clone https://github.com/sadirs/3ptWL-mod.git
    cd 3ptWL-mod
    make clean
    make all

Run the default example:

.. code-block:: bash

    cd tests
    ../wlcf

This creates run outputs such as:

* ``Output/`` for logs and used-parameter files.
* ``Bell_outputs/`` for files such as ``zetam*``, ``Bmells_*``, and ``Bnk_*``.

Documentation
-------------

Build the Sphinx documentation with:

.. code-block:: bash

    python -m pip install -r docs/requirements.txt
    make -C docs html

Open ``docs/_build/html/index.html`` after the build completes.

The Unix manual page source is available at ``docs/man/wlcf.1`` and can be
viewed with:

.. code-block:: bash

    man ./docs/man/wlcf.1

License
-------

3ptWL-mod is distributed under the MIT license. If you use this program in
research work that results in publications, please cite:

`Abraham Arvizu et al., JCAP 12 (2024) 049; arXiv:2408.16847 <https://arxiv.org/abs/2408.16847>`_

Acknowledgements
----------------

3ptWL-mod uses or builds on ideas, routines, or conventions from:

* `FFTLog <https://github.com/xfangcosmo/2DFFTLog>`_
* `The BiHaloFit model of Takahashi <https://cosmo.phys.hirosaki-u.ac.jp/takahasi/codes_e.htm>`_
* `Zeno <https://home.ifa.hawaii.edu/users/barnes/zeno/index.html>`_
* `Numerical Recipes <https://numerical.recipes/>`_
* `GSL <https://www.gnu.org/software/gsl/>`_
* `CLASS <https://github.com/lesgourg/class_public>`_
