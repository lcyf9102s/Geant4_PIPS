#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "detector.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"

class MyDetectorConstruction : public G4VUserDetectorConstruction
{
public:
    MyDetectorConstruction();
    ~MyDetectorConstruction();

    G4LogicalVolume *GetScoringVolume() const { return fScoringVolume; }
    G4LogicalVolume *GetScintScoringVolume() const { return fScoringVolumeScint; }

    virtual G4VPhysicalVolume *Construct();

private:
    virtual void ConstructSDandField();

    G4int nCols, nRows, ncomponents;
    G4Material *SiO2, *H2O, *Aerogel, *worldMat, *Air_0, *NaI, *pips, *vacuum, *steel316L, *Al_mat, *Am_mat, *icruSphereMaterial;
    G4Element *C, *Na, *I;
    G4Box *solidWorld, *solidRadiator, *solidDetector, *solidVacuum, *solidwater;
    G4Tubs *solidPIPS, *solidContainer, *solidDeadLayer, *solidAm, *solidfAm;
    G4LogicalVolume *logicWorld, *logicRadiator, *logicDetector, *logicPIPS, *logicVacuum, *logicwater, *logicContainer, *logicDeadLayer, *logicAm, *logicfAm, *fScoringVolume;
    G4VPhysicalVolume *physWorld, *physRadiator, *physDetector, *physPIPS, *physVacuum, *physwater, *physContainer, *physDeadLayer, *physfAm, *physAm;
    G4double fractionmass, density;

    // NaI(Tl) scintillator: 3"x3" active crystal wrapped in a thin Al housing
    // with a small gap (representing the MgO reflector packing). Only the
    // crystal itself is scored; see fScoringVolumeScint.
    G4Tubs *solidScintillator, *solidScintHousingOuter, *solidScintHousingCavity;
    G4SubtractionSolid *solidScintHousing;
    G4LogicalVolume *logicScintillator, *logicScintHousing, *fScoringVolumeScint;
    G4VPhysicalVolume *physScintillator, *physScintHousing;

    void DefineMaterials();
    void ConstructScintillator();

    G4GenericMessenger *fMessenger;

};

#endif