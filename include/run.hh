#ifndef RUN_HH
#define RUN_HH

#include "G4UserRunAction.hh"
#include "G4Run.hh"
#include "G4AnalysisManager.hh"
#include "G4Accumulable.hh"
//#include "event.hh"



class MyRunAction : public G4UserRunAction
{
public:
    MyRunAction();
    ~MyRunAction();

    virtual void BeginOfRunAction(const G4Run*);
    virtual void EndOfRunAction(const G4Run*);
    //void AddEventAction(MyEventAction* eventAction);
    void AddHist(G4double fEdep) {
        if((fEdep > 3)&&(fEdep < 15))
    {
        int ch = ceil(((fEdep-3)*2048)/12);
        globalHistogram[ch] += 1;
    }
    };

    void AddTotalEnergy(G4double fEdep) {
        totalEnergy += fEdep;
        //G4cout << "Total energy: " << totalEnergy << G4endl;
    };

    void GetMass(G4double mass) {
        vol_mass = mass;
    };

private:
    //std::vector<MyEventAction*> fEventActions;
    std::vector<G4double> globalHistogram;
    G4double totalEnergy;
    G4double vol_mass;
    G4double minEnergy;
    G4double maxEnergy;
    G4int nBins;

    
};

#endif