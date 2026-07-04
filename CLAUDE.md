# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Geant4 Monte Carlo simulation of a PIPS (Passivated Implanted Planar Silicon) detector for alpha-particle efficiency measurements. The source is an Am-241 disc (modeled as G4_Fe, a stand-in for the encapsulation), and the primary observable is alpha-particle energy deposition in the Si active layer. A second channel scores energy deposition in a 3"x3" NaI(Tl) scintillator crystal (see below), for gamma spectroscopy. ROOT is used for post-processing and plotting.

**Known bug (fixed in this branch):** the radioactive decay time threshold (`SetTimeThresholdForRadioactiveDecay`) used to be 200 days — far shorter than Am-241's 432-year or Cs-137's 30.17-year half-life. Nuclides with a half-life above this threshold are treated as effectively stable and never decay within the simulation, so batch runs using those ion sources used to produce **zero** counts (verified empirically). It is now set to `1000*year`, comfortably covering both. Short-lived isotopes (e.g. Po-218, T½ = 3.1 min) were always unaffected.

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
2. Bins both energy columns into 2048-channel spectra (`csv_to_dat()`): PIPS over 3–15 MeV → `output_fin.dat`, scintillator over 0–2 MeV → `output_fin_Scint.dat`
3. Plots both via ROOT: `nn()` → `EnergyDeposition.png`, `nnScint()` → `EnergyDepositionScint.png`

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
- **Scintillator detector** (`ConstructScintillator()`, built at z=+100 mm): a classic 3"x3" NaI(Tl) cylindrical crystal (r=38.1 mm, halfZ=38.1 mm), the only scored volume (`fScoringVolumeScint`). Wrapped in a thin (0.5 mm) G4_Al housing via `G4SubtractionSolid`, with a 1.5 mm air gap representing the MgO reflector packing — both crystal and housing are placed as siblings directly in `logicVacuum` (no separate gap volume needed since the gap medium matches the surrounding air).

### Scoring chain

`MySteppingAction` checks two independent volumes per step:
- `volume == fScoringVolume` (PIPS) **and** `particleName == "alpha"` → `fEventAction->AddEdep(edep)`
- `volume == fScoringVolumeScint` (any particle) → `fEventAction->AddEdepScint(edep)`

`MyEventAction::EndOfEventAction` fills ROOT n-tuple columns 0 (`Edep`, PIPS) and 1 (`EdepScint`, scintillator), and calls `fRunAction->AddHist(fEdep)`.

`MyRunAction::AddHist` bins energy into `globalHistogram[2048]` over 3–15 MeV, written to `output.dat` at end of run. **This path is broken under MT**: `BeginOfRunAction`/`EndOfRunAction` also run on the master thread's own `MyRunAction` instance, whose `globalHistogram` is never filled (the master doesn't process events) — its `EndOfRunAction` runs last and overwrites `output.dat` with all zeros. Verified empirically. The n-tuple/CSV path below is the reliable one; don't add new features via `globalHistogram`.

The n-tuple is written per-thread to `output_nt_Scoring_t<N>.csv` by Geant4's analysis manager (as `Edep,EdepScint` rows), then merged in `main()`.

### Physics list (physics.cc)

Registered physics: `G4EmStandardPhysics`, `G4OpticalPhysics`, `G4DecayPhysics`, `G4RadioactiveDecayPhysics`. Radioactive decay time threshold is set in `main()` via `SetTimeThresholdForRadioactiveDecay`, now `1000*year` (see the Known bug note above).

### Macro files

| Macro | Purpose |
|-------|---------|
| `run1.mac` | Source at z=−0.5 mm, radius 3 mm |
| `run3.mac` | Source at z=−1.5 mm, radius 3.15 mm (matches Am-241 geometry) |
| `run3_3.mac`, `run3_6.mac`, `run3_12.mac` | Variants of run3 |
| `run3v.mac` | run3 variant |
| `run2.mac` | Alternative configuration |
| `run_cs137_scint.mac` | Cs-137 ion source (z=+50 mm) aimed at the scintillator; decays via Ba-137m to the 661.7 keV gamma line. No PIPS-relevant emission |
| `vis.mac` | Interactive visualization settings |
| `vis2.mac` | Alternate visualization |

All batch macros use 16 threads (`/run/numberOfThreads 16`) and `G4GeneralParticleSource` with the ion type and position set via `{Znum}`, `{Anum}`, `{NumberOfParticles}` aliases.
