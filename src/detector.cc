#include "detector.hh"
#include "run.hh"

MySensitiveDetector::MySensitiveDetector(G4String name) : G4VSensitiveDetector(name)
{}

MySensitiveDetector::~MySensitiveDetector()
{}

G4bool MySensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *ROhist)
{
    G4Track *track = aStep->GetTrack(); // 访问粒子径迹
    track->SetTrackStatus(fStopAndKill); // 仅记录从探测器正面进入的光子（即z=480的面）

    G4StepPoint *preStepPoint = aStep->GetPreStepPoint(); // 光子进入探测器灵敏体积的位置
    G4StepPoint *postStepPoint = aStep->GetPostStepPoint(); // 光子离开探测器灵敏体积的位置

    G4ThreeVector posPhoton = preStepPoint->GetPosition();

    // G4cout << "Photon Position" << posPhoton << G4endl;
    const G4VTouchable *touchable = aStep->GetPreStepPoint()->GetTouchable(); // 获取光子进入探测器时对应的探测器位置
    G4int copyNo = touchable->GetCopyNumber(); // 将探测器位置转换为探测器单元编号
    // G4cout << "CopyNumber :" << copyNo << G4endl; // 输出相关信息

    G4VPhysicalVolume *physVol = touchable->GetVolume();
    G4ThreeVector posDetector = physVol->GetTranslation(); 
    //G4cout << "Detector pos: " << posDetector << G4endl; // 输出对应探测器的坐标

    //G4AnalysisManager *man = G4AnalysisManager::Instance();

    //G4int evt = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    //man->FillNtupleIColumn(0, evt);
    //man->FillNtupleDColumn(1, posDetector[0]);
    //man->FillNtupleDColumn(2, posDetector[1]);
    //man->FillNtupleDColumn(3, posDetector[2]);
    //man->AddNtupleRow(0);

    return 0;
}
