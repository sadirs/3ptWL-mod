# wlcf: Weak Lensing Correlation Function modeling

`wlcf` is a C code for modeling weak-lensing correlation functions in
cosmology. The current public workflow focuses on multipoles of the
three-point correlation function (3PCF) of the weak-lensing convergence field
using perturbation-theory, EFT, and Takahashi/Halo-model inspired bispectrum
branches.

The main documentation lives in `docs/` and can be built with Sphinx. A Unix
manual page is also available at `docs/man/wlcf.1`.

## Authors

- Alejandro Aviles (ICF-UNAM, Mexico), avilescervantes@gmail.com, aviles@icf.unam.mx
- Juan Carlos Hidalgo (ICF-UNAM, Mexico), hidalgo@icf.unam.mx
- Eladio A. Moreno-Alcala (UG, Mexico), ea.morenoalcala@ugto.mx
- Gustavo Niz-Quevedo (UG, Mexico), g.niz@ugto.mx
- Sadi Ramirez (ICF-UNAM, Mexico), sadi@icf.unam.mx
- Mario A. Rodriguez-Meza (ININ, Mexico), marioalberto.rodriguezmeza@gmail.com
- Sofia del Pilar Samario-Nava (ICF-UNAM, Mexico), sabiduria_sofy@hotmail.com

## Repository Layout

- `source/`, `include/`, `main/`: C implementation and executable entry point.
- `getparam/`: command-line and parameter-file parser definitions.
- `python/`: Cython wrapper source for `wlcfpy`.
- `input/`: example input power spectra and weak-lensing kernels.
- `tests/`: curated notebooks for an example WLCF run, emulator training, and
  a compact emulator-based MCMC demonstration.
- `docs/`: Sphinx documentation and manual-page sources.
- `addons/`: optional bundled or helper components such as GSL, FFTW3 notes,
  CLASS-style parser utilities, and Cython support files.

## Requirements

The C executable requires:

- A C compiler, normally `gcc`.
- GSL when `USEGSL = 1` in `Makefile_settings`.
- FFTW3 when `USEFFTW3ON = 1` in `Makefile_machine`.

The Python wrapper additionally requires Python. The build metadata installs
the Python build dependencies (`numpy`, `cython`, `setuptools`, and `wheel`)
when `pip install .` is run.

The notebooks additionally use `camb`, `scipy`, `scikit-learn`, `matplotlib`,
`emcee`, and `corner`. The Firecrown bridge notebook also uses `firecrown` and
`sacc` when those optional packages are installed. Firecrown is distributed
through `conda-forge`, so a practical notebook environment can be created with:

```bash
conda create -n wlcf-firecrown -c conda-forge \
  python=3.11 camb scipy scikit-learn matplotlib emcee corner \
  firecrown sacc jupyterlab ipykernel
conda activate wlcf-firecrown
python -m ipykernel install --user --name wlcf-firecrown \
  --display-name "Python (wlcf-firecrown)"
```

Edit `Makefile_settings` and `Makefile_machine` only when you need to change
compiler, OpenMP, GSL, or FFTW settings. The Python wrapper setup searches
environment variables, `pkg-config`, Conda, Homebrew, and common Linux library
paths automatically. For custom installs, set `GSL_DIR`, `FFTW_DIR`,
`WLCF_INCLUDE_DIRS`, or `WLCF_LIBRARY_DIRS`.

## Build

From the repository root:

```bash
make clean
make all
```

`make all` builds the `wlcf` executable, `libwlcf.a`, and the `wlcfpy` Python
wrapper. To build only the executable and static library, use:

```bash
make clean
make
```

If you need a specific Python interpreter:

```bash
PYTHON=python3 make all
```

## Run A Smoke Test

From the repository root:

```bash
make clean
make all
cd tests
../wlcf
```

The default run writes outputs under the selected output directories, normally:

- `Output/` for logs and the used-parameter file.
- `Bell_outputs/` for 3PCF and bispectrum output tables.

After each run, a `*-usedvalues` file is written and can be used as a template
for future parameter files.

## Parameters

Command-line parameters use `key=value` with no spaces:

```bash
../wlcf tree_level=4 ps=./input/linear_pk_Takahashi_z0.txt verb=2
```

To see the live parameter list, defaults, and aliases:

```bash
../wlcf --help
```

The source of truth for compiled defaults is `getparam/cmdline_defs.h`; the
Sphinx page `docs/params.rst` mirrors the main user-facing subset.

## Input And Output

The linear power-spectrum input selected by `fnamePS`/`ps` is a plain numeric
two-column ASCII table:

```text
1.000000e-03  2.345678e+04
2.000000e-03  1.987654e+04
5.000000e-03  1.234567e+04
```

When `Wg=1`, the weak-lensing kernel selected by `fWgchi` is also a plain
numeric two-column ASCII table containing `chi` and `Wg(chi)`.

Typical output files include `theta_array.txt`, `ellArray.txt`, `kArray.txt`,
`zetam*.txt`, `Bmells_*.txt`, `Bnk_*.txt`, `background_functions.txt`, and
`info.txt`, all with the configured `prefix` when applicable.

## Python

After `make all` succeeds:

```python
from wlcfpy import wlcf

w = wlcf()
w.set(
    numberThreads=8,
    tree_level=4,
    fnamePS="./input/linear_pk_Takahashi_z0.txt",
)
w.Run()
```

If the wrapper fails to build, first check that `libwlcf.a` was built and that
GSL/FFTW are visible through `pkg-config` or the environment variables listed
above.

## Hands-On Notebooks

The documented repository keeps only the notebooks needed for a clean user
workflow:

- `tests/example.ipynb`: build a linear power spectrum, run `wlcfpy`, and plot
  3PCF multipoles.
- `tests/emulator.ipynb`: generate a 300-point Latin-hypercube cosmology
  grid at the zs9 redshift, run WLCF, train one neural-network emulator per
  multipole, and save the emulator weights.
- `tests/use_wlcf_emulator.ipynb`: load the trained emulator, predict a test
  cosmology, run a small `emcee` fit with an artificial covariance, and produce
  a triangular plot.
- `tests/firecrown_emulator_likelihood.ipynb`: wrap the trained emulator as a
  Gaussian likelihood, using an artificial covariance for now and an optional
  Firecrown `Statistic`/`ConstGaussian` scaffold for later sampler integration.

Run the emulator notebooks in this order:

```bash
cd tests
jupyter lab emulator.ipynb
jupyter lab use_wlcf_emulator.ipynb
jupyter lab firecrown_emulator_likelihood.ipynb
```

Generated WLCF outputs, emulator weights, vectors, and MCMC products are ignored
by Git and can be regenerated from the notebooks.

For a GitHub upload, start from a clean checkout or remove the ignored output
directories before archiving the repository.

## Documentation

Install the documentation dependencies and build the HTML docs:

```bash
python -m pip install -r docs/requirements.txt
make -C docs html
```

Open `docs/_build/html/index.html` after the build completes. Generated docs
are intentionally ignored by `.gitignore`.

The manual page can be viewed from the repository root with:

```bash
man ./docs/man/wlcf.1
```

If `man2html` is installed, `docs/man/makehtml.sh` can generate an HTML
version of the manual page.

## Citation

If you use this code in research work that results in publications, please cite:

Abraham Arvizu et al., [JCAP 12 (2024) 049; arXiv:2408.16847](https://arxiv.org/abs/2408.16847).

## License

`wlcf` is open source and distributed under the [MIT license](LICENSE).

## Acknowledgements

`wlcf` uses or builds on ideas, routines, or conventions from:

- [FFTLog routines by Xiao Fang](https://github.com/xfangcosmo/2DFFTLog)
- [The BiHaloFit model of Takahashi](http://cosmo.phys.hirosaki-u.ac.jp/takahasi/codes_e.htm)
- [Zeno](https://home.ifa.hawaii.edu/users/barnes/zeno/index.html)
- [Numerical Recipes](https://numerical.recipes/)
- [GSL](https://www.gnu.org/software/gsl/)
- [CLASS](https://github.com/lesgourg/class_public)

Alejandro Aviles acknowledges support by grants UNAM PAPIIT IA101825 and
SECIHTI CBF2023-2024-162.
