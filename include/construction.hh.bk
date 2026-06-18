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

class MyDetectorConstruction : public G4VUserDetectorConstruction
{
public:
    MyDetectorConstruction();
    ~MyDetectorConstruction();

    G4LogicalVolume *GetScoringVolume() const { return fScoringVolume; }

    virtual G4VPhysicalVolume *Construct();

private:
    virtual void ConstructSDandField();

    G4int nCols, nRows, ncomponents;
    G4Material *SiO2, *H2O, *Aerogel, *worldMat, *Air_0, *NaI, *HPGe, *pips, *vacuum;
    G4Element *C, *Na, *I;
    G4Box *solidWorld, *solidRadiator, *solidDetector, *solidVacuum;
    G4Tubs *solidScintillator, *solidPIPS, *solidHPGe;
    G4LogicalVolume *logicWorld, *logicRadiator, *logicDetector, *logicScintillator, *logicPIPS, *logicHPGe, *logicVacuum, *fScoringVolume;
    G4VPhysicalVolume *physWorld, *physRadiator, *physDetector, *physScintillator, *physPIPS, *physHPGe, *physVacuum;
    G4double fractionmass, density;

    void DefineMaterials();

    G4GenericMessenger *fMessenger;
    
};

#endif