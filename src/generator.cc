#include "generator.hh"
#include <Randomize.hh>

#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"
#include "G4Geantino.hh"
#include "G4IonTable.hh"


MyPrimaryGenerator::MyPrimaryGenerator()
{
    fParticleGun = new G4GeneralParticleSource();
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    delete fParticleGun;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    

    fParticleGun->GeneratePrimaryVertex(anEvent);
}
