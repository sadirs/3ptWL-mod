# 3ptWL-mod: Weak-Lensing Three-Point Modeling

[![Documentation Status](https://readthedocs.org/projects/3ptwl-mod/badge/?version=latest)](https://3ptwl-mod.readthedocs.io/en/latest/?badge=latest)

**3ptWL-mod** computes theoretical multipoles of the three-point correlation
function (3PCF) of projected scalar fields, with a focus on weak-lensing
convergence. The C pipeline projects matter-bispectrum models into
\(\zeta_m(\theta_1,\theta_2)\) and supports perturbation-theory,
effective-field-theory, and Takahashi/Halo-model inspired branches.

Documentation: [3ptWL-mod on Read the Docs](https://3ptwl-mod.readthedocs.io/en/latest/)

## Authors

- Alejandro Aviles (ICF-UNAM, Mexico), avilescervantes@gmail.com, aviles@icf.unam.mx
- Juan Carlos Hidalgo (ICF-UNAM, Mexico)
- Eladio Moreno (UGTO, Mexico)
- Gustavo Niz (UGTO, Mexico)
- Mario A. Rodriguez-Meza (ININ, Mexico)
- Sofía Samario (ICF-UNAM, Mexico), ssamario@icf.unam.mx
- Sadi Ramírez (ICF-UNAM, Mexico), sadi@icf.unam.mx

## Install the Python package

3ptWL-mod builds a native extension, so install a C compiler, GSL, FFTW3,
`make`, and the Python headers first. On Debian or Ubuntu:

```bash
sudo apt-get update
sudo apt-get install build-essential libgsl-dev libfftw3-dev python3-dev pkg-config
python3 -m pip install 3ptWL-mod
```

The distribution is named `3ptWL-mod`; the import remains `wlcfpy` for
compatibility:

```bash
python3 -c "from wlcfpy import wlcf; print(wlcf)"
```

## Build the command-line tools from source

To also build the `wlcf` executable and `libwlcf.a` static library, clone the
repository and build all public interfaces:

```bash
git clone https://github.com/sadirs/3ptWL-mod.git
cd 3ptWL-mod
make clean
make PYTHON=python3 all
```

Run a compact validation calculation:

```bash
./wlcf rootDir=Output_quick prefix=quick_ \
  fnamePS=./input/linear_pk_Takahashi_z0.txt \
  numberThreads=1 verbose=0 verbose_log=0 \
  mMax=2 Nell=32 chiQuadSteps=40 GLpoints=24 writevectors=false
```

All generated files are written beneath `rootDir`. File names use `prefix`
where applicable.

## Python wrapper

After installing with pip or running `make all`:

```python
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
model.Run()
model.clean_all()
```

## Tutorials and emulator

Run the standalone four-model comparison directly in Google Colab (no
repository clone required):

[![Open In Colab](https://colab.research.google.com/assets/colab-badge.svg)](https://colab.research.google.com/github/sadirs/3ptWL-mod/blob/main/examples/3ptWL_mod_four_models_colab.ipynb)

The curated workflows live under `tests/`:

- `example.ipynb` runs the model and visualizes 3PCF multipoles;
- `emulator.ipynb` generates a cosmology grid and trains neural-network
  surrogates;
- `use_wlcf_emulator.ipynb` demonstrates accelerated inference;
- `firecrown_emulator_likelihood.ipynb` provides an experimental likelihood
  integration scaffold;
- `emulator.py` contains the reusable data-generation and training helpers.

The emulator is an optional research workflow, not a replacement for
convergence testing against direct 3ptWL-mod calculations. Its design follows
the broader neural-network acceleration strategy demonstrated by Sadi Ramirez
et al. in [*Full shape cosmology analysis from BOSS in configuration space
using neural network acceleration*](https://doi.org/10.1088/1475-7516/2024/08/049),
JCAP **2024** (08) 049.

## Repository layout

- `source/`, `include/`, `main/`: numerical C implementation and entry point.
- `getparam/`: command-line and parameter-file parser.
- `general_lib/`: shared low-level C utilities.
- `python/`: Cython wrapper and generated-declaration template.
- `input/`: example power spectra and lensing kernels.
- `tests/`: notebooks, emulator helpers, and validation inputs.
- `docs/`: Sphinx and manual-page sources.
- `addons/`: CLASS-style parser, Cython, and optional build components.

## Documentation

```bash
python3 -m pip install -r docs/requirements.txt
make -C docs html
```

Open `docs/_build/html/index.html`. Read the Docs uses the repository-level
`.readthedocs.yaml` configuration and treats Sphinx warnings as build errors.

## Citation

For the 3PCF modeling framework, cite:

- Abraham Arvizu et al., [*Modeling the 3-point correlation function of
  projected scalar fields on the sphere*](https://arxiv.org/abs/2408.16847),
  JCAP **12** (2024) 049.

When the neural-network workflow is material to the analysis, also cite:

- Sadi Ramirez, Miguel Icaza-Lizaola, Sebastien Fromenteau, Mariana
  Vargas-Magaña, and Alejandro Aviles,
  [*Full shape cosmology analysis from BOSS in configuration space using neural
  network acceleration*](https://doi.org/10.1088/1475-7516/2024/08/049),
  JCAP **2024** (08) 049.

Record the repository commit, compiler and dependency versions, and complete
run parameters in scientific releases.

## License

3ptWL-mod is distributed under the MIT license. See [LICENSE](https://github.com/sadirs/3ptWL-mod/blob/main/LICENSE).

## Acknowledgements

We acknowledge financial support from grants DGAPA-PAPIIT IA101825 and SECIHITI CBF2023-2024-162.
