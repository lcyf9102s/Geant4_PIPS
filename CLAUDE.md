# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Geant4 Monte Carlo simulation of a PIPS (Passivated Implanted Planar Silicon) detector for alpha-particle efficiency measurements. The source is an Am-241 disc (modeled as G4_Fe, a stand-in for the encapsulation), and the primary observable is alpha-particle energy deposition in the Si active layer. ROOT is used for post-processing and plotting.

## Build

Out-of-source CMake build is required. Geant4 and ROOT must be sourced before building.

```bash
# Source Geant4 and ROOT environments (paths are system-specific)
source /path/to/geant4/bin/geant4.sh
source /path/to/root/bin/thisroot.sh

mkdir build && cd build
cmake ..
make -j$(nproc)
```

The executable is named `G4Decay` and is placed in the build directory. The `.mac` files are copied there automatically by CMake.

## Running

**Interactive (visualization):**
```bash
./G4Decay
# Loads vis.mac automatically; opens OGL viewer
```

**Batch mode** (4 positional arguments required):
```bash
./G4Decay <macro.mac> <N_particles> <Z> <A>
# Example: Am-241 (Z=95, A=241), 1000 events using run3.mac
./G4Decay run3.mac 1000 95 241
```

The macro receives `{Znum}`, `{Anum}`, and `{NumberOfParticles}` as aliases set by `main()`.

After the run, `main()` automatically:
1. Merges per-thread CSV files: `tail -n +6 -q output_nt_Scoring_t*.csv >> merge.csv`
2. Bins energies into 2048-channel spectrum (`csv_to_dat()`)
3. Saves `output_fin.dat` and plots `EnergyDeposition.png` via ROOT (`nn()`)

**Note:** `main()` calls `system("rm *.png *.dat *.csv")` at startup — all previous output is wiped each run.

## Architecture

The simulation follows the standard Geant4 mandatory class pattern:

| File | Class | Role |
|------|-------|------|
| `src/construction.cc` | `MyDetectorConstruction` | Geometry and materials |
| `src/physics.cc` | `MyPhysicsList` | Physics processes |
| `src/action.cc` | `MyActionInitialization` | Wires all user actions |
| `src/generator.cc` | `MyPrimaryGenerator` | Wraps `G4GeneralParticleSource` |
| `src/stepping.cc` | `MySteppingAction` | Scores alpha energy in PIPS volume |
| `src/event.cc` | `MyEventAction` | Accumulates per-event Edep |
| `src/run.cc` | `MyRunAction` | Writes histogram and CSV output |
| `src/detector.cc` | `MySensitiveDetector` | Kills tracks on hit (legacy, not used for scoring) |
| `g4decay.cc` | `main()` + helpers | Entry point, batch control, ROOT post-processing |

### Geometry (construction.cc)

- **World** (4 m box, G4_AIR) → **Vacuum** inner box (3.2 m, G4_AIR) → all geometry placed inside Vacuum
- **Am-241 source**: two coaxial `G4Tubs` of G4_Fe — a disc (`solidAm`, r=20 mm, dz=2 mm at z=−2.5 mm) plus an annular frame (`solidfAm`, r=8–20 mm, dz=1.5 mm at z=−0.75 mm)
- **Dead layer**: Si `G4Tubs`, r=10 mm, dz=50 nm at z=+0.05 µm (entrance window)
- **PIPS active layer** (`solidPIPS`, `fScoringVolume`): Si `G4Tubs`, r=10 mm, dz=650 µm at z=0.325 mm + 0.05 µm
- An ICRU sphere (tissue-equivalent sphere) is placed at z=+1 m (for dosimetry cross-checks, not the primary observable)
- A 5×5 logical detector array at z≈2 m is constructed but not used for scoring (legacy from photon studies)

### Scoring chain

`MySteppingAction` → fires only when `volume == fScoringVolume` **and** `particleName == "alpha"` → calls `fEventAction->AddEdep(edep)`.

`MyEventAction::EndOfEventAction` → fills ROOT n-tuple column `Edep` and calls `fRunAction->AddHist(fEdep)`.

`MyRunAction::AddHist` bins energy into `globalHistogram[2048]` over the range 3–15 MeV (channel = ⌈(E−3)×2048/12⌉). Written to `output.dat` at end of run.

The n-tuple is also written per-thread to `output_nt_Scoring_t<N>.csv` by Geant4's analysis manager, then merged in `main()`.

### Physics list (physics.cc)

Registered physics: `G4EmStandardPhysics_option4`, `G4OpticalPhysics`, `G4DecayPhysics`, `G4RadioactiveDecayPhysics`. Radioactive decay time threshold set to 200 days in `main()`.

**EM physics variant:** uses `G4EmStandardPhysics_option4` rather than the plain `G4EmStandardPhysics`, since this project is a low-energy spectroscopy/thin-layer application (keV-MeV gammas, alpha stopping in a 650 µm Si layer) rather than a high-energy collider use case. `option4` adds Doppler-broadened Compton scattering, tighter step-size limits (down to 1 µm for ions vs. no explicit limit in the plain variant), safety-plus multiple-scattering step limiting near boundaries, and a lower lowest-electron-energy threshold (100 eV) — all more relevant here than in the default variant. Note: `G4RadioactiveDecayPhysics::ConstructProcess()` already calls `G4EmParameters::SetAuger(true)`, which internally also sets fluorescence on — so atomic deexcitation (fluorescence + Auger) is active globally regardless of which EM variant is registered, as long as `G4RadioactiveDecayPhysics` is present (verified against the Geant4 11.3.2 source).

**Region-based production cuts:** `construction.cc` defines a `G4Region("PIPSThinLayers")` containing `logicDeadLayer` (50 nm) and `logicPIPS` (650 µm), with the production cut tightened to 1 µm (vs. the global 1 mm default). The default cut is 1-4 orders of magnitude coarser than these two volumes, so secondary electrons/photons below the cut energy were never explicitly tracked inside them — this affected the shape of the alpha energy-loss straggling near the entrance window and active-layer boundary. Measured effect (200000-event `run3_3.mac`/Po-218 comparison, isolated via a physics-variant-only test run): the region cut alone shifts the PIPS alpha photopeak from channel 487 (5.85 MeV) to channel 387 (5.27 MeV) — about a 10% downward shift, since delta rays that previously stayed below the production-cut threshold (and so deposited their energy locally) now often exceed the 1 µm cut and can escape the thin volume before fully depositing. The `option4` EM variant alone (without the region cut) had negligible effect on this same observable.

**PIPS dead-layer energy calibration:** the combined effect of Fe self-absorption (source encapsulation), the Si dead layer, and the region-cut boundary above shifts the raw PIPS peak substantially below the true alpha energy (Am-241: 5.486 MeV true vs. 4.8633 MeV raw measured with `run3.mac`, ~11% low). `csv_to_dat()` in `g4decay.cc` applies a quadratic calibration (`PIPS_calib_a`/`PIPS_calib_b`/`PIPS_calib_c`, `E_true = a*E_meas^2 + b*E_meas + c`) before binning. An initial 2-point linear fit (Am-241, Po-218) was superseded by a 12-isotope sweep spanning 4.08-8.78 MeV (Th-232, U-238, Ra-226, Pu-239, Po-210, Am-241, Rn-222, Cm-244, Po-218, Po-216, Po-214, Po-212, all via `run3.mac`), which showed the dead-layer loss isn't perfectly linear over this wide a range: RMS residual drops from 49.0 keV (linear) to 20.7 keV (quadratic). See the README's "PIPS dead-layer energy calibration" section for the full per-isotope table and residual plot. This only rescales the display channel axis — the raw Geant4 deposit and `Total energy` remain unaffected. Note: this branch's 200-day decay threshold means most of these isotopes (T½ > 200 days) won't decay under `main`'s normal settings — the calibration sweep temporarily raised the threshold, then reverted it back to 200 days afterward.

### Macro files

| Macro | Purpose |
|-------|---------|
| `run1.mac` | Source at z=−0.5 mm, radius 3 mm |
| `run3.mac` | Source at z=−1.5 mm, radius 3.15 mm (matches Am-241 geometry) |
| `run3_3.mac`, `run3_6.mac`, `run3_12.mac` | Variants of run3 |
| `run3v.mac` | run3 variant |
| `run2.mac` | Alternative configuration |
| `vis.mac` | Interactive visualization settings |
| `vis2.mac` | Alternate visualization |

All batch macros use 16 threads (`/run/numberOfThreads 16`) and `G4GeneralParticleSource` with the ion type and position set via `{Znum}`, `{Anum}`, `{NumberOfParticles}` aliases.
