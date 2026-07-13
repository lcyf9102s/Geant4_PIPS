#include "physics.hh"


MyPhysicsList::MyPhysicsList()
{
    RegisterPhysics(new G4EmStandardPhysics_option4());
    RegisterPhysics(new G4OpticalPhysics());
    RegisterPhysics(new G4DecayPhysics());
    RegisterPhysics(new G4RadioactiveDecayPhysics());
}

MyPhysicsList::~MyPhysicsList()
{}

