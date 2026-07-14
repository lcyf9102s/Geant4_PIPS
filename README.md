# Geant4_PIPS

Geant4 Monte Carlo simulation of a PIPS (Passivated Implanted Planar Silicon) detector for alpha-particle detection efficiency measurements. The primary source is modeled as an Am-241 disc, and the main observable is the alpha-particle energy deposition spectrum in the Si active layer. Post-processing and plotting are handled via ROOT.

## Features

- Full radioactive decay chain simulation using `G4RadioactiveDecayPhysics`
- Realistic PIPS detector geometry: 50 nm dead layer + 650 µm active Si layer
- Am-241 source geometry: cylindrical disc + annular frame (G4_Fe encapsulation)
- Multi-threaded simulation (16 threads via Geant4 MT)
- Automatic post-processing: per-thread CSV merge → 2048-channel spectrum → PNG plot
- Electronics broadening model: Gaussian smearing with Fano noise + configurable electronics noise FWHM
- PIPS dead-layer energy calibration: linear correction (derived from two known alpha lines) mapping raw deposited energy back to true incident energy

## Dependencies

- [Geant4](https://geant4.org/) (tested with 11.3.2), built with `ui_all` and `vis_all`
- [ROOT](https://root.cern/) (tested with 6.x)
- CMake ≥ 3.16
- C++17 or later

## Build

Out-of-source CMake build is required. Source Geant4 and ROOT environments before building.

```bash
source /path/to/geant4/bin/geant4.sh
source /path/to/root/bin/thisroot.sh

mkdir build && cd build
cmake ..
make -j$(nproc)
```

The executable `G4Decay` and all `.mac` files are placed in the build directory.

## Usage

### Interactive mode (visualization)

```bash
./G4Decay
```

Loads `vis.mac` automatically and opens the OGL viewer.

> **Wayland users:** if the visualization window appears blank, set the following before running:
> ```bash
> export XDG_SESSION_TYPE=x11
> ```

### Batch mode

```bash
./G4Decay <macro.mac> <N_events> <Z> <A>
```

| Argument | Description |
|---|---|
| `macro.mac` | Macro file defining source geometry and run settings |
| `N_events` | Number of primary ions to simulate |
| `Z` | Atomic number of the source isotope |
| `A` | Mass number of the source isotope |

Example — 10000 Am-241 decays using `run3.mac`:
```bash
./G4Decay run3.mac 10000 95 241
```

Example — 10000 Po-218 decays using `run3_3.mac` (3 spot sources):
```bash
./G4Decay run3_3.mac 10000 84 218
```

> **Note:** each run starts by deleting all `*.png`, `*.dat`, and `*.csv` files in the working directory.

### Output files

| File | Description |
|---|---|
| `output_nt_Scoring_t*.csv` | Per-thread n-tuple (raw Edep per event, MeV) |
| `merge.csv` | Merged n-tuple from all threads |
| `output.dat` | 2048-channel histogram from `MyRunAction` (no smearing) |
| `output_fin.dat` | Final spectrum after electronics broadening (channel, counts) |
| `total_energy.dat` | Total deposited energy and kerma in the scoring volume |
| `EnergyDeposition.png` | Plot of `output_fin.dat` |

## Macro files

| Macro | Source position | Source radius | Notes |
|---|---|---|---|
| `run3.mac` | z = −1.5 mm | 3.15 mm | Single disc source, matches Am geometry |
| `run3_3.mac` | z = −1.5 mm | 3 spot sources at r = 3.4 mm | Triangular arrangement |
| `run1.mac` | z = −0.5 mm | 3 mm | Alternative distance |
| `run2.mac` | z = −11 mm | — | Gamma sources (3-spot, not for alpha runs) |
| `run3_6.mac` | z = −11 mm | 6 spot sources | Sources outside Am geometry — alphas blocked |
| `run3_12.mac` | z = −11 mm | 12 spot sources | Sources outside Am geometry — alphas blocked |
| `vis.mac` | — | — | Interactive visualization |

All batch macros use 16 threads and `G4GeneralParticleSource`. The isotope and event count are passed via command-line aliases `{Znum}`, `{Anum}`, `{NumberOfParticles}`.

## Detector geometry

```
z = −2.5 mm   Am-241 disc (G4_Fe, r = 20 mm, dz = 2 mm)
z = −0.75 mm  Am-241 frame (G4_Fe, annular r = 8–20 mm, dz = 1.5 mm)
z =  0 mm     Si dead layer (50 nm)
z = +0.325 mm Si active layer / scoring volume (650 µm, r = 10 mm)  ← Edep recorded here
```

All volumes are placed in air (G4_AIR). An ICRU sphere at z = +1 m is included for dosimetry cross-checks.

## Electronics broadening

The post-processing step applies a Gaussian smearing to each event energy to simulate the combined detector and electronics resolution:

$$\mathrm{FWHM}(E) = \sqrt{\mathrm{FWHM_{noise}}^2 + 2.355^2 \cdot F \cdot \varepsilon \cdot E}$$

| Parameter | Value | Description |
|---|---|---|
| FWHM_noise | 15 keV (default) | Electronics noise contribution |
| F | 0.12 | Si Fano factor |
| ε | 3.62 eV | Si ionization energy per e-h pair |

The parameter `FWHM_noise` can be adjusted in `csv_to_dat()` in [g4decay.cc](g4decay.cc).

## PIPS dead-layer energy calibration

Alphas lose energy before reaching the active layer — self-absorption in the Am-241 source's own Fe encapsulation, the 50 nm Si dead layer, and delta rays escaping the 1 µm production-cut boundary (see [Region-based production cuts](CLAUDE.md) for why that cut matters). For Am-241's main line this loss is substantial: ~0.6 MeV out of 5.486 MeV (~11%), so the raw deposited-energy peak reads low. This isn't a simulation bug — real PIPS alpha spectrometers show the exact same effect and are calibrated the same way: measure known lines, fit a linear map from raw pulse height back to true energy.

`csv_to_dat()` applies this calibration before binning:

$$E_{\text{true}} = \mathrm{slope} \cdot E_{\text{meas}} + \mathrm{offset}, \quad \mathrm{slope} = 1.241106,\ \mathrm{offset} = -0.549870\ \mathrm{MeV}$$

Derived from two lines simulated with `run3.mac`'s exact source geometry: Am-241 (true 5.486 MeV, measured peak 4.8633 MeV) and Po-218 (true 6.0023 MeV, measured peak 5.2793 MeV). Validated against a third, independent case — `run3_3.mac`'s Po-218 peak (a different source geometry, 3 spot sources vs. one disc) — which the calibration was **not** fit to: calibrated peak lands at 6.0000 MeV vs. the true 6.0023 MeV, 2.3 keV off. The dead-layer loss turns out to be only weakly sensitive to the small angular spread between these macros' source geometries, so a single calibration generalizes reasonably well across them — but it's still specific to this detector geometry and should be re-derived if the dead-layer thickness, source encapsulation, or production cut changes.

`PIPS_calib_slope`/`PIPS_calib_offset` in `csv_to_dat()` are the two adjustable constants. This only rescales the channel axis for display — it does not touch the raw Geant4 energy deposit, so `Total energy` in the run summary remains the true raw deposited energy. Note: since `main`'s decay threshold is still 200 days (see Physics list below), Am-241 (T½ = 432 years) won't actually decay here — the calibration was cross-checked on this branch using Po-218 instead.

### Example spectra

10000 Po-218 decays simulated with `run3_3.mac`, shown at three `FWHM_noise` settings:

| FWHM_noise = 15 keV (default) | FWHM_noise = 150 keV | FWHM_noise = 500 keV |
|---|---|---|
| ![FWHM 15 keV](docs/images/spectrum_fwhm_015.png) | ![FWHM 150 keV](docs/images/spectrum_fwhm_150.png) | ![FWHM 500 keV](docs/images/spectrum_fwhm_500.png) |

As `FWHM_noise` increases, the alpha peak flattens and broadens while the total event count is conserved.

## Physics list

| Module | Purpose |
|---|---|
| `G4EmStandardPhysics_option4` | Electromagnetic interactions (ionization, multiple scattering) — the high-precision variant, appropriate for this project's low-energy spectroscopy/thin-layer geometry rather than the plain default |
| `G4OpticalPhysics` | Optical photon transport |
| `G4DecayPhysics` | Particle decays |
| `G4RadioactiveDecayPhysics` | Radioactive decay chains |

The radioactive decay time threshold is set to 200 days in `main()` via `SetTimeThresholdForRadioactiveDecay`. This value must be adjusted to match the nuclide being simulated — it should be significantly larger than the half-life of the primary isotope. For example, Am-241 (T½ = 432 years) requires a threshold of several hundred to thousands of years, while Po-218 (T½ = 3.05 min) works fine with 200 days.
