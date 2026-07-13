# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Geant4 Monte Carlo simulation of a PIPS (Passivated Implanted Planar Silicon) detector for alpha-particle efficiency measurements. The source is an Am-241 disc (modeled as G4_Fe, a stand-in for the encapsulation), and the primary observable is alpha-particle energy deposition in the Si active layer. A second channel scores energy deposition in an HPGe crystal (see below), primarily for gamma spectroscopy. ROOT is used for post-processing and plotting.

**Fixed bug:** the radioactive decay time threshold (`SetTimeThresholdForRadioactiveDecay` in `main()`) used to be 200 days — far shorter than Am-241's 432-year or Cs-137's 30.17-year half-life. Nuclides with a half-life above this threshold are treated as effectively stable and never decay within the simulation, so batch runs using those ion sources used to produce **zero** counts (verified empirically; not a geometry issue). It is now set to `1000*year`, comfortably covering both. Short-lived isotopes (e.g. Po-218, T½ = 3.1 min) were always unaffected.

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
1. Merges per-thread CSV files: `tail -n +7 -q output_nt_Scoring_t*.csv >> merge.csv` (skips the 6-line G4 CSV header — 4 fixed lines + one `#column` line per n-tuple column; currently 2 columns)
2. Bins both energy columns into 2048-channel spectra (`csv_to_dat()`): PIPS over 3–15 MeV → `output_fin.dat`, HPGe over 0–3 MeV → `output_fin_HPGe.dat`
3. Plots both via ROOT (`nn()` → `EnergyDeposition.png`, `nnHPGe()` → `EnergyDepositionHPGe.png`)

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
- **HPGe detector** (`ConstructHPGe()`, built at z=+100 mm): a p-type closed-end coaxial crystal, G4_Ge, Rc=30 mm, Lc=60 mm, with a 5 mm-radius, 50 mm-deep bore opening at the back face. Modeled as two nested `G4SubtractionSolid`s sharing one origin:
  - `solidHPGeBody` (the whole crystal minus the bore) is the mother volume, material G4_Ge.
  - `solidHPGeActive` (`fScoringVolumeHPGe`), placed as its daughter at the same origin, is the body shrunk by a 0.3 µm outer p+ contact and a 0.5 mm n+ bore liner — only this volume is scored.
  - Housed in a G4_Al end cap (`solidHPGeEndCap`) with a 5 mm vacuum gap on all sides (reusing the previously-unused `Al_mat`/`vacuum` materials declared in `DefineMaterials()`).

### Scoring chain

`MySteppingAction` checks two independent volumes per step:
- `volume == fScoringVolume` (PIPS) **and** `particleName == "alpha"` → `fEventAction->AddEdep(edep)`
- `volume == fScoringVolumeHPGe` (any particle) → `fEventAction->AddEdepHPGe(edep)`

`MyEventAction::EndOfEventAction` fills ROOT n-tuple columns 0 (`Edep`, PIPS) and 1 (`EdepHPGe`, HPGe), and calls `fRunAction->AddHist(fEdep)`.

`MyRunAction::AddHist` bins energy into `globalHistogram[2048]` over 3–15 MeV, written to `output.dat` at end of run. **This path is broken under MT**: `BeginOfRunAction`/`EndOfRunAction` also run on the master thread's own `MyRunAction` instance, whose `globalHistogram` is never filled (the master doesn't process events) — its `EndOfRunAction` runs last and overwrites `output.dat` with all zeros. Verified empirically. The n-tuple/CSV path below is the reliable one; don't add new features via `globalHistogram`.

The n-tuple is written per-thread to `output_nt_Scoring_t<N>.csv` by Geant4's analysis manager (as `Edep,EdepHPGe` rows), then merged in `main()`.

### Physics list (physics.cc)

Registered physics: `G4EmStandardPhysics_option4`, `G4OpticalPhysics`, `G4DecayPhysics`, `G4RadioactiveDecayPhysics`. Radioactive decay time threshold set to 1000 years in `main()` (see the Fixed bug note above).

**EM physics variant:** uses `G4EmStandardPhysics_option4` rather than the plain `G4EmStandardPhysics`, since this project is a low-energy spectroscopy/thin-layer application (keV-MeV gammas, alpha stopping in a 650 µm Si layer) rather than a high-energy collider use case. `option4` adds Doppler-broadened Compton scattering, tighter step-size limits (down to 1 µm for ions vs. no explicit limit in the plain variant), safety-plus multiple-scattering step limiting near boundaries, and a lower lowest-electron-energy threshold (100 eV) — all more relevant here than in the default variant. Note: `G4RadioactiveDecayPhysics::ConstructProcess()` already calls `G4EmParameters::SetAuger(true)`, which internally also sets fluorescence on — so atomic deexcitation (fluorescence + Auger) is active globally regardless of which EM variant is registered, as long as `G4RadioactiveDecayPhysics` is present (verified against the Geant4 11.3.2 source).

**Region-based production cuts:** `construction.cc` defines a `G4Region("PIPSThinLayers")` containing `logicDeadLayer` (50 nm) and `logicPIPS` (650 µm), with the production cut tightened to 1 µm (vs. the global 1 mm default). The default cut is 1-4 orders of magnitude coarser than these two volumes, so secondary electrons/photons below the cut energy were never explicitly tracked inside them — this affected the shape of the alpha energy-loss straggling near the entrance window and active-layer boundary. Measured effect (200000-event `run3_3.mac`/Po-218 comparison, isolated via a physics-variant-only test run): the region cut alone shifts the PIPS alpha photopeak from channel 487 (5.85 MeV) to channel 387 (5.27 MeV) — about a 10% downward shift, since delta rays that previously stayed below the production-cut threshold (and so deposited their energy locally) now often exceed the 1 µm cut and can escape the thin volume before fully depositing. The `option4` EM variant alone (without the region cut) had negligible effect on this same observable. The HPGe channel is unaffected either way (no region cut applied there; the bulk Ge crystal is not thin on the relevant secondary-particle length scale).

### Macro files

| Macro | Purpose |
|-------|---------|
| `run1.mac` | Source at z=−0.5 mm, radius 3 mm |
| `run3.mac` | Source at z=−1.5 mm, radius 3.15 mm (matches Am-241 geometry) |
| `run3_3.mac`, `run3_6.mac`, `run3_12.mac` | Variants of run3 |
| `run3v.mac` | run3 variant |
| `run2.mac` | Alternative configuration |
| `run_hpge_gamma.mac` | Direct 59.5 keV gamma source aimed at the HPGe crystal, bypassing ion/decay physics — for validating HPGe geometry/scoring in isolation |
| `run_cs137_hpge.mac` | Cs-137 ion source (z=+50 mm) aimed at HPGe; decays via Ba-137m to the 661.7 keV gamma line. No PIPS-relevant emission |
| `vis.mac` | Interactive visualization settings |
| `vis2.mac` | Alternate visualization |

All batch macros use 16 threads (`/run/numberOfThreads 16`) and `G4GeneralParticleSource` with the ion type and position set via `{Znum}`, `{Anum}`, `{NumberOfParticles}` aliases.
