#include "stepping.hh"
#include "G4MTRunManager.hh"

MySteppingAction::MySteppingAction(MyEventAction *eventAction)
{
    fEventAction = eventAction;
    
}

MySteppingAction::~MySteppingAction()
{}

void MySteppingAction::UserSteppingAction(const G4Step *step)
{

    G4LogicalVolume *volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume();

    const MyDetectorConstruction *detectorConstruction = static_cast<const MyDetectorConstruction*> (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
    G4LogicalVolume *fScoringVolume = detectorConstruction->GetScoringVolume();
    G4String particleName = step->GetTrack()->GetDefinition()->GetParticleName();

    if(volume == fScoringVolume && particleName == "alpha" )
    {
        G4double edep = step->GetTotalEnergyDeposit();
        fEventAction->AddEdep(edep);
        //G4cout << "Energy deposition: " << edep / MeV << " MeV" << G4endl;
        G4double mat_dense = step->GetTrack()->GetMaterial()->GetDensity();
        G4double volume = step->GetTrack()->GetVolume()->GetLogicalVolume()->GetSolid()->GetCubicVolume();
        fEventAction->vmasscalc(mat_dense, volume);
    }
    //G4double edep = step->GetTotalEnergyDeposit();
    //fEventAction->AddEdep(edep);
}