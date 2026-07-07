#include "event.hh"
#include <fstream>


MyEventAction::MyEventAction(MyRunAction* runAction)
{
    fEdep = 0.;
    fEdepScint = 0.;
    fRunAction = runAction;


}

MyEventAction::~MyEventAction()
{}

void MyEventAction::BeginOfEventAction(const G4Event*)
{
    fEdep = 0.;
    fEdepScint = 0.;
}

void MyEventAction::EndOfEventAction(const G4Event*)
{
    // G4cout << "Energy deposition: " << fEdep << G4endl;

    //G4AnalysisManager *man = G4AnalysisManager::Instance();

    fRunAction->AddHist(fEdep);
    fRunAction->AddTotalEnergy(fEdep);
    fRunAction->GetMass(vmass);

    auto analysisManager = G4AnalysisManager::Instance();

    // Fill the n-tuple with the accumulated energy
    analysisManager->FillNtupleDColumn(0, fEdep); // Column ID 0: PIPS (alpha)
    analysisManager->FillNtupleDColumn(1, fEdepScint); // Column ID 1: scintillator
    analysisManager->AddNtupleRow();

    //man->FillNtupleDColumn(4, fEdep);
    //man->AddNtupleRow(0);
    //man->FillH1(0, fEdep);
}


