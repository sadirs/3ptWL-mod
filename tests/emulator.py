#!/usr/bin/env python3
"""Standalone WLCF emulator workflow.

This module is the single source of truth for the two notebooks:

* ``emulator.ipynb`` generates WLCF vectors and trains weights.
* ``use_wlcf_emulator.ipynb`` loads those weights and runs a small MCMC demo.

The workflow writes all generated files under ``tests/`` so the repository can
start clean and users can regenerate the emulator products themselves.
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable, Dict, Optional, Sequence, Union

import numpy as np
import pandas as pd
from scipy.optimize import least_squares
from sklearn.model_selection import train_test_split
from sklearn.neural_network import MLPRegressor


ARC_MIN_PER_RAD = 180.0 / np.pi * 60.0

RANGE_KEYS = {
    "Omega_m": "omegam_range",
    "omegam": "omegam_range",
    "h": "h_range",
    "logAs": "logAs_range",
}

PLANCK_2018_REFERENCE = {
    "Omega_m": 0.315,
    "h": 0.674,
    "logAs": 3.044,
}

ParameterInput = Union[Dict[str, float], Sequence[float], np.ndarray]

NEW_BINNING_THETA_DATA_RAD = np.array([
    0.00232712, 0.00275673, 0.00326565, 0.00386852, 0.00458269,
    0.00542871, 0.00643091, 0.00761812, 0.00902451, 0.01069053,
    0.01266412, 0.01500206, 0.01777160, 0.02105243, 0.02493894,
    0.02954293, 0.03499688, 0.04145769, 0.04911122, 0.05817769,
], dtype=float)

NEW_BINNING_THETA_BIN_CENTER_RAD = np.array([
    0.00216205, 0.00256118, 0.00303400, 0.00359411, 0.00425762,
    0.00504363, 0.00597474, 0.00707774, 0.00838437, 0.00993221,
    0.01176580, 0.01393790, 0.01651099, 0.01955910, 0.02316992,
    0.02744733, 0.03251441, 0.03851693, 0.04562757, 0.05405092,
], dtype=float)

NEW_BINNING_THETA_BIN_CENTER_ARCMIN = ARC_MIN_PER_RAD * NEW_BINNING_THETA_BIN_CENTER_RAD


@dataclass
class EmulatorConfig:
    omegam_range: tuple[float, float] = (0.20, 0.40)
    h_range: tuple[float, float] = (0.60, 0.80)
    logas_range: tuple[float, float] = (2.10, 4.00)
    grid_mode: str = "latin_hypercube"
    grid_label: str = "zs9_allm"
    n_evaluations: int = 300
    moments: tuple[int, ...] = tuple(range(9))
    seed: int = 12345
    interpolate_to_new_binning: bool = True

    default_omb: float = 0.046
    default_omc: float = 0.233
    omnu: float = 0.0
    ns: float = 0.97
    w0: float = -1.0
    z_pk: float = 0.5078
    tree_level: int = 4

    number_threads: int = 16
    nell: int = 128
    chi_quad_steps: int = 120
    gl_points: int = 32
    ellmax: float = 10000.0
    ellmin: float = 0.001
    wlcf_run_mode: str = "subprocess"
    wlcf_timeout_seconds: int = 240
    resume_existing: bool = True
    require_all_evaluations: bool = True
    stop_on_failure: bool = False
    skip_sample_ids: set[int] = field(default_factory=set)

    use_pca_targets: bool = False
    pca_variance: float = 0.99995
    hidden_layer_sizes: tuple[int, ...] = (256, 128, 64)
    mlp_max_iter: int = 3000
    mlp_alpha: float = 1.0e-8
    mlp_learning_rate_init: float = 3.0e-4
    mlp_batch_size: int = 64
    mlp_n_iter_no_change: int = 300
    mlp_tol: float = 1.0e-8
    mlp_verbose: bool = True

    split_test_size: float = 0.30
    validation_split_from_temp: float = 0.50

    points_per_moment_for_mcmc: int = 120
    selected_index_seed: int = 24680
    random_test_seed: int = 20260608
    mcmc_fractional_error: float = 0.03
    mcmc_error_floor: float = 1.0e-13
    n_walkers: int = 32
    n_steps: int = 500
    burn_in: int = 150

    @property
    def dataset_tag(self) -> str:
        return f"{self.grid_mode}_{self.grid_label}_{self.n_evaluations}"

    @property
    def baryon_fraction(self) -> float:
        return self.default_omb / (self.default_omb + self.default_omc)

    @property
    def param_columns(self) -> list[str]:
        return ["Omega_m", "h", "logAs"]


@dataclass
class EmulatorPaths:
    repo_root: Path
    test_dir: Path
    output_dir: Path
    pk_root_dir: Path
    emulator_dir: Path
    vector_dir: Path
    pk_dir: Path
    grid_path: Path
    failed_samples_path: Path
    runtime_log_path: Path
    weights_path: Path
    usage_output_dir: Path


@dataclass
class TrainingData:
    X: np.ndarray
    y: np.ndarray
    sample_ids: np.ndarray
    X_train: np.ndarray
    X_val: np.ndarray
    X_test: np.ndarray
    y_train: np.ndarray
    y_val: np.ndarray
    y_test: np.ndarray
    ids_train: np.ndarray
    ids_val: np.ndarray
    ids_test: np.ndarray
    theta_arcmin: np.ndarray
    target_shape: tuple[int, int, int]
    moments: tuple[int, ...]
    param_columns: tuple[str, ...]


@dataclass
class TrainingResult:
    model: object
    data: TrainingData
    param_mean: np.ndarray
    param_std: np.ndarray
    target_scale: np.ndarray
    target_mean: np.ndarray
    target_std: np.ndarray
    y_test_pred: np.ndarray
    rel_error: np.ndarray
    sample_error: np.ndarray
    weights_path: Path
    training_report: list[dict[str, object]] = field(default_factory=list)


def find_repo_root(start: Path | None = None) -> Path:
    if start is None:
        start = Path.cwd()
    candidates = [start, *start.parents]
    for path in candidates:
        if (path / "Makefile").exists() and (path / "source").exists() and (path / "tests").exists():
            return path
    raise RuntimeError("Could not find the wlcf repository root.")


def make_paths(config: EmulatorConfig | None = None, repo_root: Path | None = None) -> EmulatorPaths:
    config = config or EmulatorConfig()
    default_start = Path(__file__).resolve().parents[1]
    repo_root = find_repo_root(repo_root or default_start)
    test_dir = repo_root / "tests"
    emulator_dir = test_dir / "emulator_outputs"
    pk_root_dir = test_dir / "input" / "emulator_pk"
    pk_dir = pk_root_dir / config.dataset_tag
    vector_dir = emulator_dir / f"vectors_{config.dataset_tag}"
    paths = EmulatorPaths(
        repo_root=repo_root,
        test_dir=test_dir,
        output_dir=test_dir / "Bell_outputs",
        pk_root_dir=pk_root_dir,
        emulator_dir=emulator_dir,
        vector_dir=vector_dir,
        pk_dir=pk_dir,
        grid_path=emulator_dir / f"cosmology_grid_{config.dataset_tag}.csv",
        failed_samples_path=emulator_dir / f"failed_samples_{config.dataset_tag}.csv",
        runtime_log_path=emulator_dir / f"runtime_{config.dataset_tag}.csv",
        weights_path=emulator_dir / f"wlcf_cosmology_emulator_weights_{config.dataset_tag}.json",
        usage_output_dir=test_dir / "emulator_usage_demo",
    )
    for directory in [
        paths.output_dir,
        paths.pk_root_dir,
        paths.emulator_dir,
        paths.vector_dir,
        paths.pk_dir,
        paths.usage_output_dir,
    ]:
        directory.mkdir(parents=True, exist_ok=True)
    return paths


def latin_hypercube(
    n_samples: int,
    ranges: Sequence[tuple[float, float]],
    rng: np.random.Generator,
) -> np.ndarray:
    samples = np.empty((n_samples, len(ranges)))
    for dim, (low, high) in enumerate(ranges):
        points = (np.arange(n_samples) + rng.random(n_samples)) / n_samples
        rng.shuffle(points)
        samples[:, dim] = low + points * (high - low)
    return samples


def build_cosmology_grid(
    config: EmulatorConfig,
    paths: EmulatorPaths,
    save: bool = True,
) -> pd.DataFrame:
    rng = np.random.default_rng(config.seed)
    if config.grid_mode != "latin_hypercube":
        raise ValueError(f"Only latin_hypercube is configured in this workflow; got {config.grid_mode!r}")

    design = latin_hypercube(
        config.n_evaluations,
        [config.omegam_range, config.h_range, config.logas_range],
        rng,
    )

    rows = []
    for sample_id, (omega_m, h, log_as) in enumerate(design):
        as_value = np.exp(log_as) / 1.0e10
        omb = config.baryon_fraction * omega_m
        omc = omega_m - omb - config.omnu
        rows.append({
            "sample_id": sample_id,
            "Omega_m": omega_m,
            "h": h,
            "logAs": log_as,
            "As": as_value,
            "Omb": omb,
            "Omc": omc,
            "Omnu": config.omnu,
            "ns": config.ns,
            "w": config.w0,
            "prefix": f"e{sample_id:04d}_",
            "pk_file": str(paths.pk_dir / f"linear_pk_{sample_id:04d}.txt"),
        })

    grid = pd.DataFrame(rows)
    if save:
        grid.to_csv(paths.grid_path, index=False)
    return grid


def load_cosmology_grid(paths: EmulatorPaths) -> pd.DataFrame:
    if not paths.grid_path.exists():
        raise FileNotFoundError(f"Missing grid. Run build_cosmology_grid first: {paths.grid_path}")
    return pd.read_csv(paths.grid_path)


def make_camb_linear_pk(
    row: pd.Series,
    output_file: Path,
    z_pk: float,
    kmin: float = 1.0e-4,
    kmax: float = 50.0,
    npoints: int = 1000,
) -> tuple[Path, float]:
    try:
        import camb
        from camb import model as camb_model
    except ImportError as exc:
        raise ImportError("CAMB is required for grid generation. Install it with: python -m pip install camb") from exc

    output_file = Path(output_file)
    output_file.parent.mkdir(parents=True, exist_ok=True)

    pars = camb.CAMBparams()
    pars.set_cosmology(
        H0=100.0 * row.h,
        ombh2=row.Omb * row.h**2,
        omch2=row.Omc * row.h**2,
    )
    pars.InitPower.set_params(ns=row.ns, As=row.As)
    pars.set_dark_energy(w=row.w)
    pars.set_matter_power(redshifts=[z_pk], kmax=kmax)
    pars.NonLinear = camb_model.NonLinear_none

    results = camb.get_results(pars)
    kh, _, pk = results.get_matter_power_spectrum(minkh=kmin, maxkh=kmax, npoints=npoints)
    np.savetxt(output_file, np.column_stack([kh, pk[0]]), fmt="%.10e")
    sigma8 = float(results.get_sigma8()[0])
    return output_file, sigma8


def base_wlcf_params(config: EmulatorConfig, paths: EmulatorPaths) -> dict[str, object]:
    return {
        "rootDir": str(paths.output_dir),
        "numberThreads": config.number_threads,
        "verbose": 1,
        "verbose_log": 0,
        "z": config.z_pk,
        "zbin": config.z_pk,
        "Wg": 0,
        "fWgchi": "./input/Wg_Takahashi_z05078.txt",
        "tree_level": config.tree_level,
        "mMax": max(config.moments),
        "Nell": config.nell,
        "chiQuadSteps": config.chi_quad_steps,
        "GLpoints": config.gl_points,
        "ellmax": config.ellmax,
        "ellmin": config.ellmin,
        "writevectors": 0,
        "options": "",
    }


def expected_output_files(row: pd.Series, config: EmulatorConfig, paths: EmulatorPaths) -> list[Path]:
    files = [paths.output_dir / f"{row.prefix}theta_array.txt"]
    files.extend(paths.output_dir / f"{row.prefix}zetam{moment}.txt" for moment in config.moments)
    return files


def outputs_exist(row: pd.Series, config: EmulatorConfig, paths: EmulatorPaths) -> bool:
    return all(path.exists() for path in expected_output_files(row, config, paths))


def vector_path(paths: EmulatorPaths, sample_id: int) -> Path:
    return paths.vector_dir / f"target_{int(sample_id):04d}.npz"


def cleanup_sample_outputs(row: pd.Series, paths: EmulatorPaths) -> None:
    for path in paths.output_dir.glob(f"{row.prefix}*"):
        try:
            path.unlink()
        except FileNotFoundError:
            pass


def json_ready(value):
    if isinstance(value, np.integer):
        return int(value)
    if isinstance(value, np.floating):
        return float(value)
    if isinstance(value, Path):
        return str(value)
    return value


def record_runtime_sample(
    row: pd.Series,
    paths: EmulatorPaths,
    status: str,
    wall_time: float,
    cpu_time: float = np.nan,
    message: str = "",
) -> None:
    entry = pd.DataFrame([{
        "sample_id": int(row.sample_id),
        "Omega_m": row.Omega_m,
        "h": row.h,
        "logAs": row.logAs,
        "status": status,
        "wall_time_s": wall_time,
        "cpu_time_s": cpu_time,
        "message": str(message),
    }])
    entry.to_csv(
        paths.runtime_log_path,
        mode="a",
        header=not paths.runtime_log_path.exists(),
        index=False,
    )


def record_failed_sample(row: pd.Series, paths: EmulatorPaths, stage: str, message: str) -> None:
    entry = pd.DataFrame([{
        "sample_id": int(row.sample_id),
        "Omega_m": row.Omega_m,
        "h": row.h,
        "logAs": row.logAs,
        "stage": stage,
        "message": str(message),
    }])
    entry.to_csv(
        paths.failed_samples_path,
        mode="a",
        header=not paths.failed_samples_path.exists(),
        index=False,
    )


def clear_failed_sample(sample_id: int, paths: EmulatorPaths) -> None:
    if not paths.failed_samples_path.exists():
        return
    failed = pd.read_csv(paths.failed_samples_path)
    failed = failed[failed["sample_id"].astype(int) != int(sample_id)]
    if len(failed):
        failed.to_csv(paths.failed_samples_path, index=False)
    else:
        paths.failed_samples_path.unlink()


def run_wlcf_subprocess(
    params: dict[str, object],
    timeout_seconds: int,
    cwd: Path,
) -> float:
    clean_params = {key: json_ready(value) for key, value in params.items()}
    params_path = Path(tempfile.gettempdir()) / f"{clean_params['prefix']}wlcf_params.json"
    params_path.write_text(json.dumps(clean_params))

    code = r'''
import json
import sys
from pathlib import Path
from wlcfpy import wlcf

params = json.loads(Path(sys.argv[1]).read_text())
runner = wlcf()
runner.set(params)
cpu_time = runner.Run()
print(f"__WLCF_CPU_TIME__ {float(cpu_time):.12g}", flush=True)
'''

    try:
        result = subprocess.run(
            [sys.executable, "-c", code, str(params_path)],
            cwd=str(cwd),
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=timeout_seconds,
        )
    except subprocess.TimeoutExpired as exc:
        raise TimeoutError(f"WLCF subprocess exceeded {timeout_seconds} s") from exc
    finally:
        try:
            params_path.unlink()
        except FileNotFoundError:
            pass

    output = (result.stdout or "") + "\n" + (result.stderr or "")
    if result.returncode != 0:
        tail = "\n".join(output.splitlines()[-25:])
        raise RuntimeError(f"WLCF subprocess exited with code {result.returncode}:\n{tail}")

    cpu_time = np.nan
    for line in output.splitlines():
        if line.startswith("__WLCF_CPU_TIME__"):
            cpu_time = float(line.split()[1])
            break
    return cpu_time


def run_wlcf_for_row(
    row: pd.Series,
    sigma8: float,
    config: EmulatorConfig,
    paths: EmulatorPaths,
) -> float:
    sample_id = int(row.sample_id)
    print(
        f"sample {sample_id:04d}: starting WLCF "
        f"Omega_m={row.Omega_m:.6f}, h={row.h:.6f}, logAs={row.logAs:.6f}",
        flush=True,
    )
    params = dict(
        base_wlcf_params(config, paths),
        prefix=row.prefix,
        fnamePS=row.pk_file,
        h=row.h,
        Omb=row.Omb,
        Omc=row.Omc,
        Omnu=row.Omnu,
        ns=row.ns,
        w=row.w,
        sigma8=sigma8,
    )

    start = time.perf_counter()
    try:
        if config.wlcf_run_mode != "subprocess":
            raise ValueError("This standalone workflow uses WLCF subprocess mode.")
        cpu_time = run_wlcf_subprocess(params, config.wlcf_timeout_seconds, paths.test_dir)
    except Exception as exc:
        wall_time = time.perf_counter() - start
        cleanup_sample_outputs(row, paths)
        record_runtime_sample(row, paths, "failed", wall_time, np.nan, repr(exc))
        raise

    wall_time = time.perf_counter() - start
    record_runtime_sample(row, paths, "generated", wall_time, cpu_time)
    print(
        f"sample {sample_id:04d}: WLCF finished in {wall_time:.2f} s wall "
        f"({cpu_time:.2f} s reported cpu/thread)",
        flush=True,
    )
    return cpu_time


def load_raw_target(row: pd.Series, config: EmulatorConfig, paths: EmulatorPaths) -> tuple[np.ndarray, np.ndarray]:
    theta = np.loadtxt(paths.output_dir / f"{row.prefix}theta_array.txt")
    vectors = []
    n_theta = len(theta)
    for moment in config.moments:
        matrix = np.loadtxt(paths.output_dir / f"{row.prefix}zetam{moment}.txt")
        if matrix.shape != (n_theta, n_theta):
            raise ValueError(f"Expected zeta matrix {(n_theta, n_theta)}, got {matrix.shape}")
        vectors.append(np.asarray(matrix, dtype="float32").reshape(-1))
    return np.concatenate(vectors).astype("float32"), np.asarray(theta, dtype="float32")


def new_binning_theta_bin_centers_arcmin() -> np.ndarray:
    return NEW_BINNING_THETA_BIN_CENTER_ARCMIN.copy()


def zetam_model_to_data_tht(
    model_zetam: np.ndarray,
    model_theta_rad: Optional[np.ndarray] = None,
    target_theta_rad: np.ndarray = NEW_BINNING_THETA_BIN_CENTER_RAD,
) -> np.ndarray:
    """Interpolate one WLCF zeta_m matrix to the new theta-bin centers."""
    from scipy.interpolate import RectBivariateSpline

    matrix = np.asarray(model_zetam, dtype=float)
    if matrix.ndim != 2 or matrix.shape[0] != matrix.shape[1]:
        raise ValueError(f"Expected a square zeta matrix, got {matrix.shape}")

    if model_theta_rad is None:
        model_theta_rad = np.logspace(-4, 3, matrix.shape[0])
    model_theta_rad = np.asarray(model_theta_rad, dtype=float)
    if model_theta_rad.size != matrix.shape[0]:
        raise ValueError(
            f"Theta grid has {model_theta_rad.size} values, but matrix is {matrix.shape}"
        )
    if np.any(model_theta_rad <= 0) or np.any(target_theta_rad <= 0):
        raise ValueError("Theta grids must be positive for log-theta interpolation.")

    interpolator = RectBivariateSpline(
        np.log10(model_theta_rad),
        np.log10(model_theta_rad),
        matrix,
    )
    target_logtheta = np.log10(np.asarray(target_theta_rad, dtype=float))
    return interpolator(target_logtheta, target_logtheta)


def training_target_from_raw_vector(
    target: np.ndarray,
    theta_rad: Optional[np.ndarray],
    moments: Sequence[int],
    interpolate_to_new_binning: bool = True,
) -> tuple[np.ndarray, np.ndarray, tuple[int, int, int]]:
    """Convert a generated raw WLCF vector into the emulator training target."""
    target = np.asarray(target, dtype="float32")
    moments = tuple(moments)
    if len(moments) == 0:
        raise ValueError("At least one multipole is required.")

    n_theta_native = int(round(np.sqrt(target.size / len(moments))))
    expected = len(moments) * n_theta_native * n_theta_native
    if expected != target.size:
        raise ValueError(
            f"Cannot reshape target length {target.size} into "
            f"{len(moments)} square matrices."
        )

    if theta_rad is None:
        theta_rad = np.logspace(-4, 3, n_theta_native)
    theta_rad = np.asarray(theta_rad, dtype=float)
    matrices = target.reshape(len(moments), n_theta_native, n_theta_native)

    if not interpolate_to_new_binning:
        return (
            target.astype("float32"),
            (ARC_MIN_PER_RAD * theta_rad).astype("float32"),
            (len(moments), n_theta_native, n_theta_native),
        )

    interpolated = np.stack([
        zetam_model_to_data_tht(matrix, theta_rad, NEW_BINNING_THETA_BIN_CENTER_RAD)
        for matrix in matrices
    ]).astype("float32")
    return (
        interpolated.reshape(-1).astype("float32"),
        NEW_BINNING_THETA_BIN_CENTER_ARCMIN.astype("float32"),
        interpolated.shape,
    )


def new_binning_mask(rows: int = 7, diagonals: int = 4, dim: int = 20, symm: bool = True) -> np.ndarray:
    """Mask used for the new binning z=0.5 scale cut."""
    mask = np.ones((dim, dim), dtype=bool)
    mask[:rows, :] = False
    mask[:, :rows] = False
    for i in range(dim):
        for j in range(dim):
            for diag in range(diagonals):
                if abs(i - j) == diag:
                    mask[i, j] = False
            if symm and j < i:
                mask[i, j] = False
    return mask


def generate_one_sample(
    row: pd.Series,
    config: EmulatorConfig,
    paths: EmulatorPaths,
) -> str:
    row = row.copy()
    sample_id = int(row.sample_id)
    vpath = vector_path(paths, sample_id)

    if sample_id in config.skip_sample_ids:
        record_failed_sample(row, paths, "manual-skip", "Skipped by skip_sample_ids")
        return "skipped-manual"

    if config.resume_existing and vpath.exists():
        return "skipped-vector"

    if config.resume_existing and Path(row.pk_file).exists():
        sigma8 = np.nan
    else:
        _, sigma8 = make_camb_linear_pk(row, Path(row.pk_file), config.z_pk)

    if not outputs_exist(row, config, paths):
        if np.isnan(sigma8):
            _, sigma8 = make_camb_linear_pk(row, Path(row.pk_file), config.z_pk)
        run_wlcf_for_row(row, sigma8, config, paths)

    target, theta = load_raw_target(row, config, paths)
    np.savez_compressed(
        vpath,
        target=target,
        theta=theta,
        Omega_m=row.Omega_m,
        h=row.h,
        logAs=row.logAs,
        As=row.As,
        prefix=row.prefix,
    )
    clear_failed_sample(sample_id, paths)
    return "generated"


def generate_wlcf_grid(
    config: EmulatorConfig,
    paths: EmulatorPaths,
    grid: pd.DataFrame,
    progress_every: int = 10,
) -> dict[str, int]:
    os.chdir(paths.test_dir)
    start = time.perf_counter()
    counts: dict[str, int] = {"generated": 0, "skipped-vector": 0}
    for n, row in grid.iterrows():
        try:
            status = generate_one_sample(row, config, paths)
        except Exception as exc:
            status = "failed"
            record_failed_sample(row, paths, "exception", repr(exc))
            print(f"sample {int(row.sample_id):04d}: failed with {exc!r}", flush=True)
            if config.stop_on_failure:
                raise

        counts[status] = counts.get(status, 0) + 1
        if (n + 1) % progress_every == 0 or n == 0 or status in {"failed", "skipped-manual", "generated"}:
            elapsed = (time.perf_counter() - start) / 60.0
            print(f"{n + 1:03d}/{len(grid)} complete | {counts} | elapsed {elapsed:.1f} min", flush=True)
    return counts


def load_generated_dataset(
    config: EmulatorConfig,
    paths: EmulatorPaths,
    grid: pd.DataFrame,
) -> TrainingData:
    X_rows, y_rows, sample_ids = [], [], []
    theta_arcmin, target_shape = None, None
    for _, row in grid.iterrows():
        path = vector_path(paths, int(row.sample_id))
        if not path.exists():
            continue
        payload = np.load(path)
        processed_target, processed_theta_arcmin, processed_shape = training_target_from_raw_vector(
            payload["target"],
            payload["theta"] if "theta" in payload.files else None,
            config.moments,
            interpolate_to_new_binning=config.interpolate_to_new_binning,
        )
        X_rows.append(row[config.param_columns].to_numpy(dtype="float32"))
        y_rows.append(processed_target.astype("float32"))
        sample_ids.append(int(row.sample_id))
        if theta_arcmin is None:
            theta_arcmin = processed_theta_arcmin.astype("float32")
            target_shape = tuple(int(value) for value in processed_shape)

    if not X_rows:
        raise RuntimeError("No generated target vectors found. Run generate_wlcf_grid first.")

    X = np.vstack(X_rows).astype("float32")
    y = np.vstack(y_rows).astype("float32")
    sample_ids_array = np.asarray(sample_ids, dtype=int)
    if theta_arcmin is None or target_shape is None:
        n_theta = int(np.sqrt(y.shape[1] / len(config.moments)))
        theta_arcmin = np.arange(n_theta, dtype="float32")
        target_shape = (len(config.moments), n_theta, n_theta)

    if config.require_all_evaluations:
        logged_failed_ids = set()
        if paths.failed_samples_path.exists():
            failed_log = pd.read_csv(paths.failed_samples_path)
            logged_failed_ids = set(failed_log["sample_id"].astype(int)) - set(sample_ids_array.astype(int))
        skipped_or_failed_ids = set(config.skip_sample_ids) | logged_failed_ids
        expected = config.n_evaluations - len(skipped_or_failed_ids)
        if len(X) != expected:
            raise RuntimeError(
                f"Expected {expected} usable evaluations, found {len(X)}. "
                "Finish grid generation first or disable require_all_evaluations."
            )

    X_train, X_temp, y_train, y_temp, ids_train, ids_temp = train_test_split(
        X,
        y,
        sample_ids_array,
        test_size=config.split_test_size,
        random_state=config.seed,
        shuffle=True,
    )
    X_val, X_test, y_val, y_test, ids_val, ids_test = train_test_split(
        X_temp,
        y_temp,
        ids_temp,
        test_size=config.validation_split_from_temp,
        random_state=config.seed,
        shuffle=True,
    )

    return TrainingData(
        X=X,
        y=y,
        sample_ids=sample_ids_array,
        X_train=X_train,
        X_val=X_val,
        X_test=X_test,
        y_train=y_train,
        y_val=y_val,
        y_test=y_test,
        ids_train=ids_train,
        ids_val=ids_val,
        ids_test=ids_test,
        theta_arcmin=np.asarray(theta_arcmin, dtype="float32"),
        target_shape=target_shape,
        moments=tuple(int(moment) for moment in config.moments),
        param_columns=tuple(config.param_columns),
    )


def normalize_training_data(data: TrainingData):
    param_mean = data.X_train.mean(axis=0)
    param_std = data.X_train.std(axis=0)
    param_std[param_std == 0] = 1.0
    X_train_norm = (data.X_train - param_mean) / param_std
    X_val_norm = (data.X_val - param_mean) / param_std
    X_test_norm = (data.X_test - param_mean) / param_std

    target_scale = data.y_train.std(axis=0)
    target_scale[target_scale == 0] = 1.0
    y_train_sinh = np.arcsinh(data.y_train / target_scale)
    y_val_sinh = np.arcsinh(data.y_val / target_scale)
    y_test_sinh = np.arcsinh(data.y_test / target_scale)

    target_mean = y_train_sinh.mean(axis=0)
    target_std = y_train_sinh.std(axis=0)
    target_std[target_std == 0] = 1.0

    y_train_model = (y_train_sinh - target_mean) / target_std
    y_val_model = (y_val_sinh - target_mean) / target_std
    y_test_model = (y_test_sinh - target_mean) / target_std

    return {
        "param_mean": param_mean,
        "param_std": param_std,
        "target_scale": target_scale,
        "target_mean": target_mean,
        "target_std": target_std,
        "X_train_norm": X_train_norm,
        "X_val_norm": X_val_norm,
        "X_test_norm": X_test_norm,
        "y_train_model": y_train_model,
        "y_val_model": y_val_model,
        "y_test_model": y_test_model,
    }


def inverse_target_transform(y_model: np.ndarray, target_mean: np.ndarray, target_std: np.ndarray, target_scale: np.ndarray) -> np.ndarray:
    y_sinh = y_model * target_std + target_mean
    return np.sinh(y_sinh) * target_scale


def train_emulator(
    config: EmulatorConfig,
    paths: EmulatorPaths,
    data: TrainingData,
) -> TrainingResult:
    norm = normalize_training_data(data)
    models_by_moment: dict[int, MLPRegressor] = {}
    training_report: list[dict[str, object]] = []
    y_test_model_pred = np.empty_like(norm["y_test_model"])

    for moment_index, moment in enumerate(data.moments):
        target_slice = moment_slice(moment_index, data.target_shape)
        model = MLPRegressor(
            hidden_layer_sizes=config.hidden_layer_sizes,
            activation="relu",
            solver="adam",
            alpha=config.mlp_alpha,
            learning_rate="adaptive",
            learning_rate_init=config.mlp_learning_rate_init,
            batch_size=config.mlp_batch_size,
            max_iter=config.mlp_max_iter,
            early_stopping=True,
            validation_fraction=0.15,
            n_iter_no_change=config.mlp_n_iter_no_change,
            tol=config.mlp_tol,
            random_state=config.seed + 1000 + int(moment),
            verbose=config.mlp_verbose,
        )
        if config.mlp_verbose:
            print(f"Training one emulator for multipole m={moment}")
        model.fit(norm["X_train_norm"], norm["y_train_model"][:, target_slice])

        val_pred = model.predict(norm["X_val_norm"])
        val_truth = norm["y_val_model"][:, target_slice]
        y_test_model_pred[:, target_slice] = model.predict(norm["X_test_norm"])

        models_by_moment[int(moment)] = model
        training_report.append({
            "moment": int(moment),
            "iterations": int(model.n_iter_),
            "loss": float(model.loss_),
            "val_mse_norm": float(np.mean((val_pred - val_truth) ** 2)),
            "val_mae_norm": float(np.mean(np.abs(val_pred - val_truth))),
            "output_size": int(target_slice.stop - target_slice.start),
        })

    y_test_pred = inverse_target_transform(
        y_test_model_pred,
        norm["target_mean"],
        norm["target_std"],
        norm["target_scale"],
    )
    denom = np.maximum(np.abs(data.y_test), 1.0e-11)
    rel_error = np.abs((y_test_pred - data.y_test) / denom)
    sample_error = rel_error.mean(axis=1)

    result = TrainingResult(
        model=models_by_moment,
        data=data,
        param_mean=norm["param_mean"],
        param_std=norm["param_std"],
        target_scale=norm["target_scale"],
        target_mean=norm["target_mean"],
        target_std=norm["target_std"],
        y_test_pred=y_test_pred,
        rel_error=rel_error,
        sample_error=sample_error,
        weights_path=paths.weights_path,
        training_report=training_report,
    )
    save_emulator_state(config, paths, result)
    return result


def save_emulator_state(
    config: EmulatorConfig,
    paths: EmulatorPaths,
    result: TrainingResult,
) -> Path:
    models_by_moment = result.model
    if not isinstance(models_by_moment, dict):
        raise RuntimeError("Expected one trained sklearn model per multipole.")

    models_state = []
    for moment in result.data.moments:
        model = models_by_moment.get(int(moment))
        if model is None or not hasattr(model, "coefs_"):
            raise RuntimeError(f"Train the sklearn emulator for multipole {moment} before saving it.")
        models_state.append({
            "moment": int(moment),
            "coefs": [coef.tolist() for coef in model.coefs_],
            "intercepts": [intercept.tolist() for intercept in model.intercepts_],
            "hidden_layer_sizes": list(model.hidden_layer_sizes),
            "activation": model.activation,
            "loss": float(model.loss_),
            "n_iter": int(model.n_iter_),
        })

    state = {
        "backend": "sklearn",
        "model_family": "sklearn_by_moment",
        "param_columns": config.param_columns,
        "moments": list(config.moments),
        "theta_arcmin": result.data.theta_arcmin.tolist(),
        "target_shape": list(result.data.target_shape),
        "target_representation": "full_matrix",
        "upper_triangle_only": False,
        "theta_binning": "new_binning" if config.interpolate_to_new_binning else "native_wlcf",
        "interpolate_to_new_binning": bool(config.interpolate_to_new_binning),
        "new_binning_theta_bin_center_rad": NEW_BINNING_THETA_BIN_CENTER_RAD.tolist(),
        "new_binning_theta_bin_center_arcmin": NEW_BINNING_THETA_BIN_CENTER_ARCMIN.tolist(),
        "target_transform": "arcsinh_std",
        "param_mean": result.param_mean.tolist(),
        "param_std": result.param_std.tolist(),
        "target_scale": result.target_scale.tolist(),
        "target_mean": result.target_mean.tolist(),
        "target_std": result.target_std.tolist(),
        "use_pca_targets": False,
        "pca_variance": config.pca_variance,
        "omegam_range": list(config.omegam_range),
        "h_range": list(config.h_range),
        "logAs_range": list(config.logas_range),
        "hidden_layer_sizes": list(config.hidden_layer_sizes),
        "activation": "relu",
        "training_report": result.training_report,
        "models": models_state,
    }
    with paths.weights_path.open("w") as handle:
        json.dump(state, handle)
    return paths.weights_path


def activate(x: np.ndarray, name: str) -> np.ndarray:
    if name == "relu":
        return np.maximum(x, 0.0)
    if name == "tanh":
        return np.tanh(x)
    if name == "logistic":
        return 1.0 / (1.0 + np.exp(-x))
    if name in ("identity", None):
        return x
    raise ValueError(f"Unsupported activation: {name}")


def apply_mlp(
    x: np.ndarray,
    coefs: list[np.ndarray],
    intercepts: list[np.ndarray],
    activation: str,
) -> np.ndarray:
    for layer, (coef, intercept) in enumerate(zip(coefs, intercepts)):
        x = x @ coef + intercept
        if layer < len(coefs) - 1:
            x = activate(x, activation)
    return x


def load_saved_emulator(weights_path: Path) -> dict[str, object]:
    with Path(weights_path).open("r") as handle:
        state = json.load(handle)

    emulator: dict[str, object] = {
        "raw_state": state,
        "param_columns": list(state["param_columns"]),
        "moments": list(state["moments"]),
        "target_shape": tuple(state["target_shape"]),
        "param_mean": np.asarray(state["param_mean"], dtype=float),
        "param_std": np.asarray(state["param_std"], dtype=float),
        "target_mean": np.asarray(state["target_mean"], dtype=float),
        "target_std": np.asarray(state["target_std"], dtype=float),
        "target_transform": state.get("target_transform", "arcsinh_std"),
        "target_floor": float(state.get("target_floor", 0.0)),
        "target_representation": state.get("target_representation", "full_matrix"),
        "upper_triangle_only": bool(state.get("upper_triangle_only", False)),
        "model_family": state.get("model_family", "single_mlp"),
        "activation": state.get("activation", "relu"),
        "use_pca_targets": bool(state.get("use_pca_targets", False)),
        "theta_arcmin": np.asarray(
            state.get("theta_arcmin", np.arange(state["target_shape"][-1])),
            dtype=float,
        ),
    }
    emulator["bounds"] = np.asarray(
        [state[RANGE_KEYS[name]] for name in emulator["param_columns"]],
        dtype=float,
    )
    emulator["n_theta"] = int(emulator["target_shape"][-1])
    emulator["bins_per_moment"] = int(
        state.get(
            "bins_per_moment",
            emulator["n_theta"] * (emulator["n_theta"] + 1) // 2
            if emulator["upper_triangle_only"]
            else emulator["n_theta"] * emulator["n_theta"],
        )
    )

    if emulator["model_family"] == "sklearn_by_moment":
        emulator["models"] = []
        for model_state in state["models"]:
            emulator["models"].append({
                "moment": int(model_state["moment"]),
                "coefs": [np.asarray(weight, dtype=float) for weight in model_state["coefs"]],
                "intercepts": [
                    np.asarray(bias, dtype=float) for bias in model_state["intercepts"]
                ],
                "activation": model_state.get("activation", state.get("activation", "tanh")),
            })
    else:
        emulator["coefs"] = [np.asarray(weight, dtype=float) for weight in state["coefs"]]
        emulator["intercepts"] = [
            np.asarray(bias, dtype=float) for bias in state["intercepts"]
        ]

    if emulator["target_transform"] == "arcsinh_std":
        emulator["target_scale"] = np.asarray(state["target_scale"], dtype=float)
    else:
        emulator["target_scale"] = None

    if emulator["use_pca_targets"]:
        emulator["pca_components"] = np.asarray(state["pca_components"], dtype=float)
        emulator["pca_mean"] = np.asarray(state["pca_mean"], dtype=float)
        emulator["coeff_mean"] = np.asarray(state["coeff_mean"], dtype=float)
        emulator["coeff_std"] = np.asarray(state["coeff_std"], dtype=float)

    return emulator


def inverse_saved_target_transform(y_norm: np.ndarray, emulator: dict[str, object]) -> np.ndarray:
    y_model = y_norm * emulator["target_std"] + emulator["target_mean"]
    if emulator["target_transform"] == "arcsinh_std":
        return np.sinh(y_model) * emulator["target_scale"]
    if emulator["target_transform"] == "log10_positive":
        y = np.power(10.0, y_model) - emulator["target_floor"]
        return np.maximum(y, emulator["target_floor"])
    raise ValueError(f"Unsupported target transform: {emulator['target_transform']}")


def predict_saved_native(theta: np.ndarray, emulator: dict[str, object]) -> np.ndarray:
    x = np.atleast_2d(np.asarray(theta, dtype=float))
    x = (x - emulator["param_mean"]) / emulator["param_std"]

    if emulator["model_family"] == "sklearn_by_moment":
        pieces = []
        for model_state in emulator["models"]:
            pieces.append(
                apply_mlp(
                    x.copy(),
                    model_state["coefs"],
                    model_state["intercepts"],
                    model_state["activation"],
                )
            )
        y_norm = np.concatenate(pieces, axis=1)
        return inverse_saved_target_transform(y_norm, emulator)[0]

    y_norm = apply_mlp(
        x,
        emulator["coefs"],
        emulator["intercepts"],
        emulator["activation"],
    )
    if emulator["use_pca_targets"]:
        coeff = y_norm * emulator["coeff_std"] + emulator["coeff_mean"]
        y_norm = coeff @ emulator["pca_components"] + emulator["pca_mean"]
    return inverse_saved_target_transform(y_norm, emulator)[0]


class WLCFEmulator:
    """Convenience wrapper around weights trained by this standalone workflow."""

    def __init__(self, weights_path: Path):
        self.weights_path = Path(weights_path)
        self.emulator = load_saved_emulator(self.weights_path)
        self.param_names = list(self.emulator["param_columns"])
        self.moments = list(self.emulator["moments"])
        self.bounds = np.asarray(self.emulator["bounds"], dtype=float)
        self.n_theta = int(self.emulator["n_theta"])
        self.bins_per_moment = int(self.emulator["bins_per_moment"])
        self.target_shape = tuple(self.emulator["target_shape"])
        self.target_representation = str(self.emulator["target_representation"])
        self.upper_triangle_only = bool(self.emulator["upper_triangle_only"])
        self.theta_arcmin = np.asarray(self.emulator["theta_arcmin"], dtype=float)

    def theta_array(self, params: ParameterInput) -> np.ndarray:
        if isinstance(params, dict):
            missing = [name for name in self.param_names if name not in params]
            if missing:
                raise KeyError(f"Missing parameters: {missing}")
            theta = np.asarray([params[name] for name in self.param_names], dtype=float)
        else:
            theta = np.asarray(params, dtype=float)

        if theta.shape != (len(self.param_names),):
            raise ValueError(
                f"Expected {len(self.param_names)} parameters {self.param_names}; "
                f"got shape {theta.shape}"
            )
        return theta

    def center_parameters(self) -> dict[str, float]:
        center = self.bounds.mean(axis=1)
        return dict(zip(self.param_names, center.astype(float)))

    def check_bounds(self, params: ParameterInput) -> bool:
        theta = self.theta_array(params)
        return bool(np.all(theta >= self.bounds[:, 0]) and np.all(theta <= self.bounds[:, 1]))

    def predict(self, params: ParameterInput) -> np.ndarray:
        return predict_saved_native(self.theta_array(params), self.emulator)

    def predict_vector(
        self,
        params: ParameterInput,
        moments: Optional[Sequence[int]] = None,
    ) -> np.ndarray:
        if moments is None:
            moments = self.moments
        return self.select_moments(self.predict(params), list(moments))

    def select_moments(self, vector: np.ndarray, moments: Sequence[int]) -> np.ndarray:
        vector = np.asarray(vector, dtype=float)
        moments = list(moments)

        if self.target_representation == "upper_triangle":
            expected = len(self.moments) * self.bins_per_moment
            if vector.size != expected:
                raise ValueError(f"Expected vector length {expected}, got {vector.size}")
            pieces = []
            for moment in moments:
                if moment not in self.moments:
                    raise ValueError(f"moment {moment} not available; available: {self.moments}")
                idx = self.moments.index(moment)
                start = idx * self.bins_per_moment
                pieces.append(vector[start:start + self.bins_per_moment])
            return np.concatenate(pieces)

        matrices = vector.reshape(self.target_shape)
        pieces = []
        for moment in moments:
            if moment not in self.moments:
                raise ValueError(f"moment {moment} not available; available: {self.moments}")
            pieces.append(matrices[self.moments.index(moment)].ravel())
        return np.concatenate(pieces)

    def split_vector_by_moment(
        self,
        vector: np.ndarray,
        moments: Optional[Sequence[int]] = None,
    ) -> dict[int, np.ndarray]:
        if moments is None:
            moments = self.moments
        moments = list(moments)
        vector = np.asarray(vector, dtype=float)

        if self.target_representation == "upper_triangle":
            stride = self.bins_per_moment
        else:
            stride = self.n_theta * self.n_theta

        expected = len(moments) * stride
        if vector.size != expected:
            raise ValueError(f"Expected vector length {expected}, got {vector.size}")

        return {
            int(moment): vector[i * stride:(i + 1) * stride]
            for i, moment in enumerate(moments)
        }

    def triangle_to_matrix(
        self,
        triangle_vector: np.ndarray,
        fill_lower: bool = True,
        fill_value: float = np.nan,
    ) -> np.ndarray:
        triangle_vector = np.asarray(triangle_vector, dtype=float)
        if triangle_vector.size != self.bins_per_moment:
            raise ValueError(
                f"Expected one moment with {self.bins_per_moment} entries, "
                f"got {triangle_vector.size}"
            )

        matrix = np.full((self.n_theta, self.n_theta), fill_value, dtype=float)
        mask = np.triu(np.ones((self.n_theta, self.n_theta), dtype=bool))
        matrix[mask] = triangle_vector
        if fill_lower:
            lower = np.tril(np.ones((self.n_theta, self.n_theta), dtype=bool), k=-1)
            matrix[lower] = matrix.T[lower]
        return matrix

    def predict_matrices(
        self,
        params: ParameterInput,
        moments: Optional[Sequence[int]] = None,
        fill_lower: bool = True,
    ) -> dict[int, np.ndarray]:
        if moments is None:
            moments = self.moments
        moments = list(moments)
        selected = self.predict_vector(params, moments=moments)
        pieces = self.split_vector_by_moment(selected, moments=moments)

        if self.target_representation == "upper_triangle":
            return {
                moment: self.triangle_to_matrix(piece, fill_lower=fill_lower)
                for moment, piece in pieces.items()
            }

        return {
            moment: piece.reshape(self.n_theta, self.n_theta)
            for moment, piece in pieces.items()
        }


def summarize_test_error(result: TrainingResult) -> dict[str, float]:
    return {
        "mean_relative_error": float(result.rel_error.mean()),
        "median_relative_error": float(np.median(result.rel_error)),
        "p95_relative_error": float(np.percentile(result.rel_error, 95)),
        "worst_sample_mean_error": float(result.sample_error.max()),
    }


def moment_slice(moment_index: int, target_shape: tuple[int, int, int]) -> slice:
    stride = int(target_shape[1] * target_shape[2])
    start = int(moment_index * stride)
    return slice(start, start + stride)


def vector_to_moment_matrices(
    vector: np.ndarray,
    target_shape: tuple[int, int, int],
    moments: Sequence[int],
) -> dict[int, np.ndarray]:
    matrices = np.asarray(vector, dtype=float).reshape(target_shape)
    return {int(moment): matrices[i] for i, moment in enumerate(moments)}


def test_error_by_moment(result: TrainingResult) -> pd.DataFrame:
    rows = []
    for moment_index, moment in enumerate(result.data.moments):
        target_slice = moment_slice(moment_index, result.data.target_shape)
        rel_m = result.rel_error[:, target_slice]
        sign_mismatch = np.signbit(result.y_test_pred[:, target_slice]) != np.signbit(
            result.data.y_test[:, target_slice]
        )
        rows.append({
            "moment": int(moment),
            "mean_rel": float(rel_m.mean()),
            "median_rel": float(np.median(rel_m)),
            "p95_rel": float(np.percentile(rel_m, 95)),
            "worst_sample_mean_rel": float(rel_m.mean(axis=1).max()),
            "sign_mismatch": float(sign_mismatch.mean()),
        })
    return pd.DataFrame(rows)


def plot_new_binning_mask(rows: int = 7, diagonals: int = 4):
    """Plot new theta bins and the paper-style scale-cut mask."""
    import matplotlib.pyplot as plt
    from matplotlib.colors import ListedColormap

    theta_labels = [f"{value:.0f}'" for value in NEW_BINNING_THETA_BIN_CENTER_ARCMIN]
    dim = len(theta_labels)
    upper = np.triu(np.ones((dim, dim), dtype=bool))
    mask = new_binning_mask(rows=rows, diagonals=diagonals, dim=dim, symm=True)

    fig, ax = plt.subplots(figsize=(7.0, 6.6), constrained_layout=True)
    ax.imshow(
        upper.astype(int),
        origin="lower",
        cmap=ListedColormap(["white", "#66e7e6"]),
        interpolation="nearest",
        vmin=0,
        vmax=1,
    )

    for i in range(dim + 1):
        ax.axhline(i - 0.5, color="0.55", lw=0.8)
        ax.axvline(i - 0.5, color="0.55", lw=0.8)

    for i, j in np.argwhere(mask):
        ax.text(j, i, "✓", ha="center", va="center", fontsize=13, color="black")

    ax.set_xticks(np.arange(dim))
    ax.set_yticks(np.arange(dim))
    ax.set_xticklabels(theta_labels, rotation=55, ha="right")
    ax.set_yticklabels(theta_labels)
    ax.set_xlabel(r"$\theta_1$ [arcmin]", fontsize=13)
    ax.set_ylabel(r"$\theta_2$ [arcmin]", fontsize=13)
    ax.set_title(r"$\zeta_m$ new binning mask", fontsize=15)
    ax.set_xlim(-0.5, dim - 0.5)
    ax.set_ylim(-0.5, dim - 0.5)
    return fig, mask


def plot_test_precision_summary(result: TrainingResult):
    """Plot a compact, publication-style summary of test-set emulator errors."""
    import matplotlib.pyplot as plt

    metrics_df = test_error_by_moment(result)
    fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.2), constrained_layout=True)

    ax = axes[0]
    ax.hist(result.sample_error, bins=25, color="0.20", edgecolor="white", lw=0.7)
    ax.axvline(np.median(result.sample_error), color="#d62728", lw=2.0, label="median")
    ax.axvline(np.percentile(result.sample_error, 95), color="#1f77b4", lw=2.0, ls="--", label="95%")
    ax.set_xlabel("mean relative error per cosmology")
    ax.set_ylabel("count")
    ax.set_title("Test-set error distribution")
    ax.legend(frameon=False)

    ax = axes[1]
    x = np.arange(len(metrics_df))
    ax.bar(x, metrics_df["median_rel"].to_numpy(), color="0.35", width=0.7, label="median")
    ax.plot(x, metrics_df["mean_rel"].to_numpy(), color="#1f77b4", marker="o", lw=1.7, label="mean")
    ax.plot(x, metrics_df["p95_rel"].to_numpy(), color="#d62728", marker="s", lw=1.7, label="95%")
    ax.set_xticks(x)
    ax.set_xticklabels([str(int(m)) for m in metrics_df["moment"]])
    ax.set_xlabel("multipole m")
    ax.set_ylabel("relative error")
    ax.set_title("Error by multipole")
    ax.legend(frameon=False)

    return fig, metrics_df


def plot_test_prediction_comparison(
    result: TrainingResult,
    sample_index: Optional[int] = None,
    moments: Sequence[int] = (0, 1, 2),
    zeta_limits: tuple[float, float] = (1.0e-11, 1.0e-8),
):
    """Compare WLCF target matrices against emulator predictions for one test sample."""
    import matplotlib.pyplot as plt
    from matplotlib.colors import LogNorm

    available_moments = list(result.data.moments)
    selected_moments = [int(moment) for moment in moments if int(moment) in available_moments]
    if not selected_moments:
        raise ValueError(f"No requested moments are available: {moments}")

    if sample_index is None:
        sample_index = int(np.argsort(result.sample_error)[len(result.sample_error) // 2])
    sample_index = int(np.clip(sample_index, 0, len(result.data.X_test) - 1))

    truth = result.data.y_test[sample_index]
    pred = result.y_test_pred[sample_index]
    truth_mats = vector_to_moment_matrices(truth, result.data.target_shape, available_moments)
    pred_mats = vector_to_moment_matrices(pred, result.data.target_shape, available_moments)

    theta = np.asarray(result.data.theta_arcmin, dtype=float)
    theta2, theta1 = np.meshgrid(theta, theta)
    xlim = (max(10.0, float(theta.min())), min(200.0, float(theta.max())))
    ylim = xlim

    residuals = [
        (pred_mats[moment] - truth_mats[moment]) / np.maximum(np.abs(truth_mats[moment]), 1.0e-11)
        for moment in selected_moments
    ]
    resid_limit = float(np.nanpercentile(np.abs(np.concatenate([r.ravel() for r in residuals])), 95))
    resid_limit = max(resid_limit, 1.0e-3)

    fig = plt.figure(figsize=(4.1 * len(selected_moments) + 1.0, 9.2), constrained_layout=True)
    gs = fig.add_gridspec(
        nrows=3,
        ncols=len(selected_moments) + 1,
        width_ratios=[1] * len(selected_moments) + [0.06],
    )

    zeta_mesh = None
    residual_mesh = None
    for col, moment in enumerate(selected_moments):
        panels = [
            ("WLCF", np.abs(truth_mats[moment]), "zeta"),
            ("Emulator", np.abs(pred_mats[moment]), "zeta"),
            ("Relative error", residuals[col], "residual"),
        ]
        for row, (label, matrix, kind) in enumerate(panels):
            ax = fig.add_subplot(gs[row, col])
            if kind == "zeta":
                zeta_mesh = ax.pcolormesh(
                    theta2,
                    theta1,
                    np.maximum(matrix, 1.0e-30),
                    shading="auto",
                    cmap="RdYlBu_r",
                    norm=LogNorm(vmin=zeta_limits[0], vmax=zeta_limits[1]),
                )
            else:
                residual_mesh = ax.pcolormesh(
                    theta2,
                    theta1,
                    matrix,
                    shading="auto",
                    cmap="RdBu_r",
                    vmin=-resid_limit,
                    vmax=resid_limit,
                )
            ax.set_xscale("log")
            ax.set_yscale("log")
            ax.set_xlim(*xlim)
            ax.set_ylim(*ylim)
            ax.set_aspect("equal", adjustable="box")
            if row == 0:
                ax.set_title(fr"$m={moment}$", fontsize=14)
            if col == 0:
                ax.set_ylabel(label + "\n" + r"$\theta_1$ [arcmin]", fontsize=12)
            if row == 2:
                ax.set_xlabel(r"$\theta_2$ [arcmin]", fontsize=12)

    if zeta_mesh is not None:
        cax = fig.add_subplot(gs[0:2, -1])
        cbar = fig.colorbar(zeta_mesh, cax=cax)
        cbar.set_label(r"$|\zeta_m|$", fontsize=12)
    if residual_mesh is not None:
        cax = fig.add_subplot(gs[2, -1])
        cbar = fig.colorbar(residual_mesh, cax=cax)
        cbar.set_label("rel. err.", fontsize=12)

    params = ", ".join(
        f"{name}={value:.4g}"
        for name, value in zip(result.data.param_columns, result.data.X_test[sample_index])
    )
    sample_id = int(result.data.ids_test[sample_index])
    fig.suptitle(f"Test sample {sample_id}: emulator vs WLCF ({params})", fontsize=15)
    return fig


def choose_test_sample(
    config: EmulatorConfig,
    paths: EmulatorPaths,
    emulator,
    rng: np.random.Generator | None = None,
    target_params: ParameterInput | None = None,
) -> tuple[pd.DataFrame, np.ndarray, int, np.ndarray, Path]:
    grid = load_cosmology_grid(paths)
    available_ids = {int(path.stem.split("_")[-1]) for path in paths.vector_dir.glob("target_*.npz")}
    grid = grid[grid["sample_id"].astype(int).isin(available_ids)].copy()
    grid = grid.sort_values("sample_id").reset_index(drop=True)
    if len(grid) < 10:
        raise RuntimeError("Not enough generated vectors for the test-set demo.")

    X = grid[emulator.param_names].to_numpy(float)
    sample_ids = grid["sample_id"].to_numpy(int)
    _, X_temp, _, ids_temp = train_test_split(
        X,
        sample_ids,
        test_size=config.split_test_size,
        random_state=config.seed,
        shuffle=True,
    )
    _, X_test, _, ids_test = train_test_split(
        X_temp,
        ids_temp,
        test_size=config.validation_split_from_temp,
        random_state=config.seed,
        shuffle=True,
    )

    if target_params is None:
        rng = rng or np.random.default_rng(config.random_test_seed)
        choice = int(rng.integers(0, len(ids_test)))
    else:
        target = emulator.theta_array(target_params)
        scale = np.maximum(emulator.bounds[:, 1] - emulator.bounds[:, 0], 1.0e-12)
        distance = np.linalg.norm((X_test - target) / scale, axis=1)
        choice = int(np.argmin(distance))

    sample_id = int(ids_test[choice])
    truth = X_test[choice]
    return grid, X_test, sample_id, truth, vector_path(paths, sample_id)


def load_target_vector(target_path: Path, emulator) -> np.ndarray:
    payload = np.load(target_path)
    target = np.asarray(payload["target"], dtype=float)
    expected = int(np.prod(emulator.target_shape))
    if target.size == expected:
        return target

    processed, _, _ = training_target_from_raw_vector(
        target,
        payload["theta"] if "theta" in payload.files else None,
        emulator.moments,
        interpolate_to_new_binning=bool(
            emulator.emulator["raw_state"].get("interpolate_to_new_binning", True)
        ),
    )
    if processed.size != expected:
        raise ValueError(f"Expected target length {expected}, got {processed.size}")
    return processed


def build_selected_indices(
    config: EmulatorConfig,
    emulator,
    output_path: Path | None = None,
) -> np.ndarray:
    rng = np.random.default_rng(config.selected_index_seed)
    if emulator.target_representation == "full_matrix":
        stride = emulator.n_theta * emulator.n_theta
    else:
        stride = emulator.bins_per_moment
    selected = []
    for moment in emulator.moments:
        size = min(config.points_per_moment_for_mcmc, stride)
        local = np.sort(rng.choice(stride, size=size, replace=False))
        selected.extend((emulator.moments.index(moment) * stride + local).tolist())
    selected_indices = np.asarray(selected, dtype=int)
    if output_path is not None:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        np.save(output_path, selected_indices)
    return selected_indices


def build_artificial_covariance(
    data_vector: np.ndarray,
    config: EmulatorConfig,
    variance_path: Path | None = None,
    covariance_path: Path | None = None,
) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    sigma = config.mcmc_fractional_error * np.maximum(np.abs(data_vector), config.mcmc_error_floor)
    variance = sigma**2
    covariance = np.diag(variance)
    if variance_path is not None:
        variance_path.parent.mkdir(parents=True, exist_ok=True)
        np.save(variance_path, variance)
    if covariance_path is not None:
        covariance_path.parent.mkdir(parents=True, exist_ok=True)
        np.save(covariance_path, covariance)
    return sigma, variance, covariance


def make_selected_predictor(emulator, selected_indices: np.ndarray) -> Callable[[np.ndarray], np.ndarray]:
    state = emulator.emulator

    def inverse_selected(y_norm_selected: np.ndarray) -> np.ndarray:
        target_mean = state["target_mean"][selected_indices]
        target_std = state["target_std"][selected_indices]
        y_model = y_norm_selected * target_std + target_mean
        if state["target_transform"] == "arcsinh_std":
            return np.sinh(y_model) * state["target_scale"][selected_indices]
        if state["target_transform"] == "log10_positive":
            y = np.power(10.0, y_model) - state["target_floor"]
            return np.maximum(y, state["target_floor"])
        raise ValueError(f"Unsupported target transform: {state['target_transform']}")

    def predict_selected(theta: np.ndarray) -> np.ndarray:
        theta = emulator.theta_array(theta)
        if state["model_family"] != "single_mlp":
            return emulator.predict(theta)[selected_indices]

        x = np.atleast_2d(theta)
        x = (x - state["param_mean"]) / state["param_std"]
        coefs = state["coefs"]
        intercepts = state["intercepts"]
        for coef, intercept in zip(coefs[:-1], intercepts[:-1]):
            x = activate(x @ coef + intercept, state["activation"])

        if state["use_pca_targets"]:
            coeff_norm = x @ coefs[-1] + intercepts[-1]
            coeff = coeff_norm * state["coeff_std"] + state["coeff_mean"]
            y_norm_selected = (
                coeff @ state["pca_components"][:, selected_indices]
                + state["pca_mean"][selected_indices]
            )
        else:
            y_norm_selected = x @ coefs[-1][:, selected_indices] + intercepts[-1][selected_indices]
        return inverse_selected(y_norm_selected[0])

    return predict_selected


def run_mcmc_fit(
    emulator,
    predict_selected: Callable[[np.ndarray], np.ndarray],
    data_vector: np.ndarray,
    sigma: np.ndarray,
    truth: np.ndarray,
    config: EmulatorConfig,
    rng: np.random.Generator | None = None,
) -> dict[str, object]:
    rng = rng or np.random.default_rng(config.random_test_seed)
    lower = emulator.bounds[:, 0]
    upper = emulator.bounds[:, 1]
    inv_sigma = 1.0 / sigma

    def residual(theta: np.ndarray) -> np.ndarray:
        return (predict_selected(theta) - data_vector) * inv_sigma

    def log_prior(theta: np.ndarray) -> float:
        theta = np.asarray(theta, dtype=float)
        if np.all(theta >= lower) and np.all(theta <= upper):
            return 0.0
        return -np.inf

    def log_likelihood(theta: np.ndarray) -> float:
        r = residual(theta)
        return -0.5 * float(np.dot(r, r))

    def log_prob(theta: np.ndarray) -> float:
        lp = log_prior(theta)
        if not np.isfinite(lp):
            return -np.inf
        return lp + log_likelihood(theta)

    start = np.clip(truth + rng.normal(0.0, 0.01 * (upper - lower)), lower, upper)
    lsq = least_squares(residual, start, bounds=(lower, upper), max_nfev=300)
    best_theta = lsq.x

    ndim = len(emulator.param_names)
    proposal_width = np.maximum(0.002 * (upper - lower), 1.0e-4)
    initial = best_theta + rng.normal(0.0, proposal_width, size=(config.n_walkers, ndim))
    initial = np.clip(initial, lower + 1.0e-9, upper - 1.0e-9)

    import emcee

    sampler = emcee.EnsembleSampler(config.n_walkers, ndim, log_prob)
    sampler.run_mcmc(initial, config.n_steps, progress=True)
    chain = sampler.get_chain(discard=config.burn_in, flat=True)
    raw_chain = sampler.get_chain()
    return {
        "start": start,
        "least_squares": lsq,
        "best_theta": best_theta,
        "chain": chain,
        "raw_chain": raw_chain,
        "acceptance_fraction": float(np.mean(sampler.acceptance_fraction)),
        "log_likelihood_best": log_likelihood(best_theta),
    }


def summarize_chain(chain: np.ndarray, names: Sequence[str], truth: np.ndarray | None = None) -> pd.DataFrame:
    rows = []
    for i, name in enumerate(names):
        q16, q50, q84 = np.percentile(chain[:, i], [16, 50, 84])
        row = {
            "parameter": name,
            "median": q50,
            "minus_1sigma": q50 - q16,
            "plus_1sigma": q84 - q50,
        }
        if truth is not None:
            row["truth"] = truth[i]
            row["delta_median_minus_truth"] = q50 - truth[i]
        rows.append(row)
    return pd.DataFrame(rows)


def run_all(config: EmulatorConfig | None = None) -> TrainingResult:
    config = config or EmulatorConfig()
    paths = make_paths(config)
    grid = build_cosmology_grid(config, paths)
    generate_wlcf_grid(config, paths, grid)
    data = load_generated_dataset(config, paths, grid)
    result = train_emulator(config, paths, data)
    print("Test error:", summarize_test_error(result))
    print("Weights:", result.weights_path)
    return result


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "stage",
        choices=["grid", "generate", "train", "all"],
        help="Workflow stage to run.",
    )
    args = parser.parse_args()

    config = EmulatorConfig()
    paths = make_paths(config)

    if args.stage == "grid":
        grid = build_cosmology_grid(config, paths)
        print(paths.grid_path)
        print(grid.head())
    elif args.stage == "generate":
        grid = load_cosmology_grid(paths) if paths.grid_path.exists() else build_cosmology_grid(config, paths)
        print(generate_wlcf_grid(config, paths, grid))
    elif args.stage == "train":
        grid = load_cosmology_grid(paths)
        data = load_generated_dataset(config, paths, grid)
        result = train_emulator(config, paths, data)
        print(summarize_test_error(result))
        print(result.weights_path)
    else:
        run_all(config)


if __name__ == "__main__":
    main()
