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
#include "TRandom3.h"

void plotSpectrum(const char *datFile, const char *pngFile, int nBins)
{
    using namespace std;
    TGraph *graph = new TGraph();
    graph->SetMarkerStyle(kFullCircle);

    fstream file;
    file.open(datFile, ios::in);

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

    TCanvas *canvas = new TCanvas("canvas", "Energy Deposition Spectrum", 800, 600);
    graph->Draw("AL");

    graph->GetXaxis()->SetRangeUser(0, nBins - 1);
    canvas->Update();
    canvas->SaveAs(pngFile);
    delete canvas;
}

void nn() { plotSpectrum("output_fin.dat", "EnergyDeposition.png", 2048); }
void nnScint() { plotSpectrum("output_fin_Scint.dat", "EnergyDepositionScint.png", 2048); }

// Same data as plotSpectrum, but with a log-scale y-axis, matching how
// published background spectra (wide dynamic range: tall low-energy peaks,
// small high-energy ones) are usually displayed.
void plotSpectrumLog(const char *datFile, const char *pngFile, int nBins)
{
    using namespace std;
    TGraph *graph = new TGraph();
    graph->SetMarkerStyle(kFullCircle);

    fstream file;
    file.open(datFile, ios::in);

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

    TCanvas *canvas = new TCanvas("canvasLog", "Energy Deposition Spectrum (log)", 800, 600);
    canvas->SetLogy();
    graph->Draw("AL");

    graph->GetXaxis()->SetRangeUser(0, nBins - 1);
    canvas->Update();
    canvas->SaveAs(pngFile);
    delete canvas;
}

void nnScintLog() { plotSpectrumLog("output_fin_Scint.dat", "EnergyDepositionScint_log.png", 2048); }

// Same data as plotSpectrum, but applies ROOT's built-in TH1::Smooth()
// (the "353QH, twice" algorithm) before plotting, to tame the Poisson
// counting noise visible in the scintillator's broad, low-statistics peak.
// Does not touch the underlying .dat file.
void plotSpectrumSmoothed(const char *datFile, const char *pngFile, int nBins)
{
    using namespace std;
    std::vector<double> xs, ys;

    fstream file;
    file.open(datFile, ios::in);
    while(true)
    {
        double x, y;
        file >> x >> y;
        if(file.eof()) break;
        xs.push_back(x);
        ys.push_back(y);
    }
    file.close();

    if (xs.empty()) return;

    TH1D *hist = new TH1D("hSmooth", "Energy Deposition Spectrum (smoothed)", nBins, -0.5, nBins - 0.5);
    hist->SetStats(0);
    for (size_t i = 0; i < xs.size(); ++i) {
        hist->SetBinContent(static_cast<int>(xs[i]) + 1, ys[i]);
    }
    hist->Smooth(5); // "353QH, twice", applied 5 times

    hist->GetXaxis()->SetTitle("Channel");
    hist->GetXaxis()->CenterTitle();
    hist->GetYaxis()->SetTitle("Counts");
    hist->GetYaxis()->CenterTitle();

    TCanvas *canvas = new TCanvas("canvasSmooth", "Energy Deposition Spectrum (smoothed)", 800, 600);
    hist->Draw("HIST L");
    canvas->Update();
    canvas->SaveAs(pngFile);
    delete canvas;
    delete hist;
}

void nnScintSmooth() { plotSpectrumSmoothed("output_fin_Scint.dat", "EnergyDepositionScint_smooth.png", 2048); }

void csv_to_dat(){
    std::string filename = "merge.csv"; // Your file name
    // Column 0 (Edep): PIPS, alpha only, binned over 3-15 MeV, 2048 channels.
    // Column 1 (EdepScint): NaI(Tl) scintillator, any particle, binned over 0-3 MeV, 2048 channels.
    // (Widened from 0-1 MeV so the background macro's lines up to 2.6 MeV fit.)
    const int nBinsPIPS = 2048;
    const int nBinsScint = 2048;
    const double maxEnergyScint = 3.0; // MeV
    std::vector<G4double> MCHist(nBinsPIPS, 0.0);
    std::vector<G4double> MCHistScint(nBinsScint, 0.0);

    // Electronics broadening model: FWHM(E)^2 = FWHM_noise^2 + 2.355^2 * F * eps * E
    // FWHM_noise: electronic noise (MeV), F: Si Fano factor, eps: ionization energy per e-h pair (MeV)
    const double FWHM_noise = 0.015;   // 15 keV
    const double F_fano     = 0.12;    // Si Fano factor
    const double eps_si     = 3.62e-6; // MeV per e-h pair in Si

    // NaI(Tl) resolution: photon-statistics-limited scaling, FWHM(E)/E = R_662 * sqrt(662 keV / E),
    // i.e. FWHM(E) = R_662 * sqrt(E_ref * E). ORTEC's own "Experiment 3: Gamma-Ray
    // Spectroscopy Using NaI(Tl)" derives exactly this 1/sqrt(E) scaling law (Eq. 1) from
    // photoelectron-counting statistics, and works a concrete example: a measured Cs-137
    // photopeak at channel 280 with FWHM = 32 channels, giving 11.5% resolution at 662 keV
    // (Experiment 3.4). ORTEC notes a theoretical floor of ~7.4% for a high-quality detector
    // (~1000 photoelectrons) and a typical range of 7-10%; 11.5% is their own worked example.
    const double R_662_Scint = 0.115;  // dimensionless resolution at 662 keV
    const double E_ref_Scint = 0.662;  // MeV

    // Create an input file stream object
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        //return 1;
    }

    G4double totalEnergy = 0.0;
    size_t nLines = 0;
    std::string line;
    // Read the file line by line: each row is "Edep,EdepScint"
    while (std::getline(inputFile, line)) {
        // Skip any empty lines or header lines that start with '#'
        if (line.empty() || line[0] == '#') {
            continue;
        }

        double E_pips = 0.0, E_scint = 0.0;
        try {
            size_t comma = line.find(',');
            E_pips = std::stod(line.substr(0, comma));
            if (comma != std::string::npos) {
                E_scint = std::stod(line.substr(comma + 1));
            }
        } catch (const std::invalid_argument& e) {
            std::cerr << "Warning: Could not convert line to a number: " << line << std::endl;
            continue;
        }
        ++nLines;
        totalEnergy += E_pips;

        if (E_pips > 0) {
            double fwhm2 = FWHM_noise * FWHM_noise + 5.5460 * F_fano * eps_si * E_pips;
            double sigma  = std::sqrt(fwhm2) / 2.355;
            double E_meas = gRandom->Gaus(E_pips, sigma);
            if (E_meas > 3 && E_meas < 15) {
                int ch = ceil(((E_meas - 3) * nBinsPIPS) / 12);
                MCHist[ch] += 1;
            }
        }

        if (E_scint > 0) {
            double fwhm  = R_662_Scint * std::sqrt(E_ref_Scint * E_scint);
            double sigma = fwhm / 2.355;
            double E_meas = gRandom->Gaus(E_scint, sigma);
            int ch = floor((E_meas * nBinsScint) / maxEnergyScint);
            if (ch >= 0 && ch < nBinsScint) {
                MCHistScint[ch] += 1;
            }
        }
    }

    inputFile.close();

    std::cout << "Successfully read " << nLines << " data points." << std::endl;
    G4cout << "Total energy (PIPS): " << totalEnergy * 1.6 * pow(10, -13) << " J" << G4endl;

    std::ofstream outFile("output_fin.dat");
    for(int i = 0; i < nBinsPIPS; i++)
    {
        outFile << i << " " << MCHist[i] << "\n";
    }
    outFile.close();

    std::ofstream outFileScint("output_fin_Scint.dat");
    for(int i = 0; i < nBinsScint; i++)
    {
        outFileScint << i << " " << MCHistScint[i] << "\n";
    }
    outFileScint.close();
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
    // Nuclides with a half-life above this threshold are treated as stable and
    // never decay in the simulation. 200 days was far too short for isotopes
    // like Cs-137 (T half-life = 30.17 years) or Am-241 (T half-life = 432 years),
    // which silently never decayed. 1000 years comfortably covers both.
    G4HadronicParameters::Instance()->SetTimeThresholdForRadioactiveDecay( 1000*CLHEP::year );

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
    system("tail -n +7 -q output_nt_Scoring_t*.csv >> merge.csv");
    csv_to_dat();
    nn();
    nnScint();
    nnScintLog();
    nnScintSmooth();
    }

    //return 0;
    delete visManager;
    delete runManager;
}
