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
    G4LogicalVolume *GetHPGeScoringVolume() const { return fScoringVolumeHPGe; }

    virtual G4VPhysicalVolume *Construct();

private:
    virtual void ConstructSDandField();

    G4int nCols, nRows, ncomponents;
    G4Material *SiO2, *H2O, *Aerogel, *worldMat, *Air_0, *NaI, *HPGe, *pips, *vacuum, *steel316L, *Al_mat, *Am_mat, *icruSphereMaterial;
    G4Element *C, *Na, *I;
    G4Box *solidWorld, *solidRadiator, *solidDetector, *solidVacuum, *solidwater;
    G4Tubs *solidScintillator, *solidPIPS, *solidContainer, *solidDeadLayer, *solidAm, *solidfAm;
    G4LogicalVolume *logicWorld, *logicRadiator, *logicDetector, *logicScintillator, *logicPIPS, *logicVacuum, *logicwater, *logicContainer, *logicDeadLayer, *logicAm, *logicfAm, *fScoringVolume;
    G4VPhysicalVolume *physWorld, *physRadiator, *physDetector, *physScintillator, *physPIPS, *physVacuum, *physwater, *physContainer, *physDeadLayer, *physfAm, *physAm;
    G4double fractionmass, density;

    // HPGe: p-type closed-end coaxial crystal (bore + n+ liner + p+ outer contact),
    // housed in an Al end cap with a vacuum gap. Only the active (non-contact) Ge
    // volume is scored; see fScoringVolumeHPGe.
    G4Tubs *solidHPGeEnvelope, *solidHPGeBoreMech, *solidHPGeActiveEnv, *solidHPGeBoreLiner;
    G4Tubs *solidHPGeEndCapOuter, *solidHPGeEndCapCavity;
    G4SubtractionSolid *solidHPGeBody, *solidHPGeActive, *solidHPGeEndCap;
    G4LogicalVolume *logicHPGeEndCap, *logicHPGeVacuum, *logicHPGeBody, *logicHPGeActive, *fScoringVolumeHPGe;
    G4VPhysicalVolume *physHPGeEndCap, *physHPGeVacuum, *physHPGeBody, *physHPGeActive;

    void DefineMaterials();
    void ConstructHPGe();

    G4GenericMessenger *fMessenger;

};

#endif