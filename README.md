# 3ptWL-mod: Weak-Lensing Three-Point Modeling

[![Documentation Status](https://readthedocs.org/projects/3ptwl-mod/badge/?version=latest)](https://3ptwl-mod.readthedocs.io/en/latest/?badge=latest)

**3ptWL-mod** computes theoretical multipoles of the three-point correlation
function (3PCF) of projected scalar fields, with a focus on weak-lensing
convergence. The C pipeline projects matter-bispectrum models into
\(\zeta_m(\theta_1,\theta_2)\) and supports perturbation-theory,
effective-field-theory, and Takahashi/Halo-model inspired branches.

The repository provides:

- the `wlcf` command-line executable;
- the `libwlcf.a` static library;
- the `wlcfpy` Cython wrapper;
- notebooks for 3PCF visualization and neural-network emulation.

The executable and Python package retain their historical `wlcf` names for API
compatibility. The repository name reflects the scientific role of the code.

Documentation: [3ptWL-mod on Read the Docs](https://3ptwl-mod.readthedocs.io/en/latest/)

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

## Contributors

The scientific code and documentation include contributions from Alejandro
Aviles, Juan Carlos Hidalgo, Eladio Moreno, Gustavo Niz, Sadi Ramirez, Mario A.
Rodriguez-Meza, Sofía Samario, and collaborators.

## License and acknowledgements

3ptWL-mod is distributed under the [MIT license](LICENSE). The project uses or
builds on FFTLog, BiHaloFit/Takahashi models, Zeno, Numerical Recipes, GSL, and
CLASS. Alejandro Aviles acknowledges support by grants UNAM PAPIIT IA101825 and
SECIHTI CBF2023-2024-162.
