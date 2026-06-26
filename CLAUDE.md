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

Registered physics: `G4EmStandardPhysics`, `G4OpticalPhysics`, `G4DecayPhysics`, `G4RadioactiveDecayPhysics`. Radioactive decay time threshold set to 200 days in `main()`.

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
