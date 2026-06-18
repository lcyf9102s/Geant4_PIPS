#include <iostream>
#include "G4RunManager.hh"
#include "G4MTRunManager.hh"
#include <G4UIExecutive.hh>
#include <G4VisManager.hh>
#include <G4VisExecutive.hh>
#include <G4UImanager.hh>
#include <construction.hh>
#include "physics.hh"
#include "action.hh"
#include "G4HadronicParameters.hh"
#include <cstdlib>
#include <fstream>
#include "TH1.h"
#include "TFile.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TStyle.h"

void nn()
{
    using namespace std;
    TGraph *graph = new TGraph();
    graph->SetMarkerStyle(kFullCircle);

    fstream file;
    file.open("output_fin.dat", ios::in);

    while(true)
    {
        double x, y;
        file >> x >> y;
        graph->SetPoint(graph->GetN(), x, y);
        if(file.eof()) break;
    }
    file.close();
    
    graph->GetXaxis()->SetTitle("Channel");
    graph->GetXaxis()->CenterTitle();
    graph->GetYaxis()->SetTitle("Counts");
    graph->GetYaxis()->CenterTitle();

    TCanvas *canvas = new TCanvas("canvas", "Exponential Decay Fit", 800, 600);
    graph->Draw("AL");

    graph->GetXaxis()->SetRangeUser(0, 2047);
    canvas->Update();
    canvas->SaveAs("EnergyDeposition.png");

}

void csv_to_dat(){
    std::vector<double> data_column;
    std::string filename = "merge.csv"; // Your file name
    std::vector<G4double> MCHist;
    
    MCHist.resize(2048, 0.0);
    
    // Create an input file stream object
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        //return 1;
    }

    std::string line;
    // Read the file line by line
    while (std::getline(inputFile, line)) {
        // Skip any empty lines or header lines that start with '#'
        if (line.empty() || line[0] == '#') {
            continue;
        }

        try {
            // Convert the entire line to a double and add it to the vector
            data_column.push_back(std::stod(line));
        } catch (const std::invalid_argument& e) {
            std::cerr << "Warning: Could not convert line to a number: " << line << std::endl;
        }
    }

    inputFile.close();

    // --- Verification: Print the contents of the vector ---
    std::cout << "Successfully read " << data_column.size() << " data points." << std::endl;
    //std::cout << "First 10 values: " << std::endl;
    G4double totalEnergy = 0.0;
    for (size_t i = 0; i < data_column.size(); ++i) {
        //std::cout << data_column[i] << std::endl;
        totalEnergy += data_column[i];
        if((data_column[i] > 3)&&(data_column[i] < 15))
        {
            int ch = ceil(((data_column[i]-3)*2048)/12);
            MCHist[ch] += 1;
        }
    }
    G4cout << "Total energy: " << totalEnergy * 1.6 * pow(10, -13) << " J" << G4endl;

    std::ofstream outFile("output_fin.dat");
    for(int i = 0; i < 2048; i++)
    {
        outFile << i << " " << MCHist[i] << "\n";
    }
    //outFile << "Total energy: " << totalEnergy << " Mev"<<"\n";
    outFile.close();
}


int main(int argc, char** argv)
{
    system("rm *.png *.dat *.csv");
    G4MTRunManager *runManager = new G4MTRunManager();
    //runManager->SetNumberOfThreads(12);
    //G4cout << "Multithreaded" << G4endl;
    runManager->SetUserInitialization(new MyDetectorConstruction());
    runManager->SetUserInitialization(new MyPhysicsList());
    runManager->SetUserInitialization(new MyActionInitialization());


    // runManager->Initialize();
    G4HadronicParameters::Instance()->SetTimeThresholdForRadioactiveDecay( 200*CLHEP::day );

    G4UIExecutive *ui = 0;
    if(argc == 1)
    {
        ui = new G4UIExecutive(argc, argv);
    }

    G4VisManager *visManager = new G4VisExecutive();
    visManager->Initialize();
    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    if (ui)
    {
        runManager->Initialize();  
        UImanager->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart(); 
        delete ui;
    }
    else
    {
        // batch mode
    
    G4String A_num = argv[4];
    G4String command4 = "/control/alias Anum ";
    UImanager->ApplyCommand(command4 + A_num);
    G4String Z_num = argv[3];
    G4String command3 = "/control/alias Znum ";
    UImanager->ApplyCommand(command3 + Z_num);
    
    // Set the number of particles
    G4String number = argv[2];
    G4String command1 = "/control/alias NumberOfParticles ";
    UImanager->ApplyCommand(command1 + number);
    G4String command2 = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command2 + fileName);
    //nn();
    system("tail -n +6 -q output_nt_Scoring_t*.csv >> merge.csv");
    csv_to_dat();
    nn();
    }

    //return 0;
    delete visManager;
    delete runManager;
}
