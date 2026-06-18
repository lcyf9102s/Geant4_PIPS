#include "run.hh"
#include "G4SystemOfUnits.hh"
#include <CLHEP/Units/SystemOfUnits.h>
#include <fstream>
#include <iostream>
#include "event.hh"
#include <G4LogicalVolume.hh>
#include "G4AccumulableManager.hh"


MyRunAction::MyRunAction()
{
    //G4AnalysisManager *man = G4AnalysisManager::Instance();
    //man->CreateNtuple("Hits", "Hits");
    //man->CreateNtupleIColumn("fEvent");
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->SetVerboseLevel(1);
    
    // Create an n-tuple to store the energy deposition
    analysisManager->CreateNtuple("Scoring", "Deposited Energy");
    analysisManager->CreateNtupleDColumn("Edep"); // Double-precision column for Edep
    analysisManager->FinishNtuple();
    totalEnergy = 0.0;
    vol_mass = 0.0;    
    //man->CreateNtupleDColumn("fX");
    //man->CreateNtupleDColumn("fY");
    //man->CreateNtupleDColumn("fZ");
    //man->CreateNtupleDColumn("Edep");
    //man->CreateH1("Edep", "Energy deposition", 2048, 0.*MeV, 8.*MeV);
    //man->FinishNtuple(0);
    
}

MyRunAction::~MyRunAction()
{}

void MyRunAction::BeginOfRunAction(const G4Run* run)
{
    //G4AnalysisManager *man = G4AnalysisManager::Instance();
    //man->CreateH1("Edep", "Energy deposition", 2048, 0.*MeV, 8.*MeV);
    //G4int runID = run->GetRunID();
    //std::stringstream strRunID;
    //strRunID << runID;
    auto analysisManager = G4AnalysisManager::Instance();
    
    // Get the run number and construct the output file name
    G4int runID = run->GetRunID();
    G4String fileName = "output.csv";
    
    // Open the output file
    analysisManager->OpenFile(fileName);
    
    nBins = 2048;
    globalHistogram.resize(nBins, 0.0);

    //man->OpenFile("output"+strRunID.str()+".root");

}



void MyRunAction::EndOfRunAction(const G4Run* endrun)
{
    //G4AnalysisManager *man = G4AnalysisManager::Instance();
    
    std::ofstream outFile("output.dat");
    for(int i = 0; i < 2048; i++)
    {
        outFile << i << " " << globalHistogram[i] << "\n";
    }
    //outFile << "Total energy: " << totalEnergy << " Mev"<<"\n";
    outFile.close();

    std::ofstream totalEnergyFile("total_energy.dat");
    totalEnergyFile << "Total energy: " << totalEnergy << " MeV" << "\n";
    totalEnergyFile << "Kerma in Volume: " << totalEnergy * (1.6 * pow(10, -13))/vol_mass << " Gy" << "\n";
    G4cout << "Scoring volume mass: " << vol_mass / g << " g" << G4endl;
    totalEnergyFile.close();
    //man->Write();
    //man->CloseFile();
    auto analysisManager = G4AnalysisManager::Instance();
    
    // Write the n-tuple to the file and close it
    analysisManager->Write();
    analysisManager->CloseFile();
    
}

//void MyRunAction::AddEventAction(MyEventAction* eventAction) {
//    fEventActions.push_back(eventAction);
//}
