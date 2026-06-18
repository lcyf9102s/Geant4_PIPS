#include "generator.hh"
#include <Randomize.hh>

MyPrimaryGenerator::MyPrimaryGenerator()
{
    fParticleGun = new G4ParticleGun(1);
    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    //G4String particleName = "gamma";
    //G4ParticleDefinition *particle = particleTable->FindParticle("gamma");
    G4ParticleDefinition *particle = particleTable->FindParticle("geantino");
    G4ThreeVector pos(0., 0., 0.);

    

    fParticleGun->SetParticlePosition(pos);
    
    //fParticleGun->SetParticleMomentum(100* G4UniformRand() *keV);
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
    fParticleGun->SetParticleMomentum(0.*keV);
    fParticleGun->SetParticleDefinition(particle);
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    delete fParticleGun;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    
    // 发射方向随机化
    G4double theta = CLHEP::pi * G4UniformRand();
    G4double _phi = 2 * CLHEP::pi * G4UniformRand();
    G4double x = std::sin(theta) * std::cos(_phi);
    G4double y = std::sin(theta) * std::sin(_phi);
    G4double z = std::cos(theta);

    G4double cosTheta = 2*G4UniformRand() - 1., phi = CLHEP::twopi*G4UniformRand();
    G4double sinTheta = std::sqrt(1. - cosTheta*cosTheta);
    G4double ux = sinTheta*std::cos(phi),
            uy = sinTheta*std::sin(phi),
            uz = cosTheta;
    
    G4ParticleDefinition *particle = fParticleGun->GetParticleDefinition();
    if(particle == G4Geantino::Geantino())
    {
        G4int Z = 84, A = 218;
        G4double charge = 0.*eplus;
        G4double energy = 0.*keV;

        G4ParticleDefinition *ion = G4IonTable::GetIonTable()->GetIon(Z, A, energy);
        fParticleGun->SetParticleDefinition(ion);
        fParticleGun->SetParticleCharge(charge);

    }
    
    fParticleGun->GeneratePrimaryVertex(anEvent);
}
