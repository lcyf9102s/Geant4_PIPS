#include "construction.hh"

MyDetectorConstruction::MyDetectorConstruction()
{
    
    fMessenger = new G4GenericMessenger(this, "/detector/", "Detector hh");
    fMessenger->DeclareProperty("nCol", nCols, "Number of columns");
    fMessenger->DeclareProperty("nRows", nRows, "Number of rows");
    nCols = 5;
    nRows = 5;
    DefineMaterials();

}

void MyDetectorConstruction::DefineMaterials()
{
    
    G4NistManager *nist = G4NistManager::Instance();
    Air_0 = nist->FindOrBuildMaterial("G4_AIR");
    worldMat = nist->FindOrBuildMaterial("G4_AIR"); // ROI(boundary) & material
    
    // Define Vacuum
    vacuum = new G4Material("Vacuum", density= 1.e-5*g/cm3, ncomponents=1, kStateGas, 153*kelvin, 1.e-8*atmosphere);
    vacuum->AddMaterial(Air_0, fractionmass=1.);
    // 设置产生切伦科夫辐射的介质构成：气凝胶（aerogel）SiO2, H2O, C
    SiO2 = new G4Material("SiO2", 2.201*g/cm3, 2);
    SiO2->AddElement(nist->FindOrBuildElement("Si"), 1);
    SiO2->AddElement(nist->FindOrBuildElement("O"), 2);

    H2O = new G4Material("H2O", 1.000*g/cm3, 2);
    H2O->AddElement(nist->FindOrBuildElement("H"), 2);
    H2O->AddElement(nist->FindOrBuildElement("O"), 1);

    NaI = new G4Material("NaI", 3.67*g/cm3, 2);
    NaI->AddElement(nist->FindOrBuildElement("Na"), 1);
    NaI->AddElement(nist->FindOrBuildElement("I"), 1);

    HPGe = nist->FindOrBuildMaterial("G4_Ge");

    pips = nist->FindOrBuildMaterial("G4_Si");


    

    C = nist->FindOrBuildElement("C");

    Aerogel = new G4Material("Aerogel", 0.200*g/cm3, 3);
    Aerogel->AddMaterial(SiO2, 62.5*perCent);
    Aerogel->AddMaterial(H2O, 37.4*perCent);
    Aerogel->AddElement(C, 0.1*perCent);
    // define materials
    // World材质
    // 设置切伦科夫辐射相关参数，momentum动量、refractive index(?)折射率（要显示可见光光子，必须要设定材料的反射系数）
    G4double energy[2] = {1.239841939*eV/0.9, 1.239841939*eV/0.2};
    G4double rindexAerogel[2] = {1.1, 1.1};
    G4MaterialPropertiesTable *mptAerogel = new G4MaterialPropertiesTable();
    mptAerogel->AddProperty("RINDEX", energy, rindexAerogel, 2);
    Aerogel->SetMaterialPropertiesTable(mptAerogel);

    // 设置world的折射率，以显示光路
    G4double rindexWorld[2] = {1.0, 1.0};
    G4MaterialPropertiesTable *mptWorld = new G4MaterialPropertiesTable();
    mptWorld->AddProperty("RINDEX", energy, rindexWorld, 2);
    //worldMat->SetMaterialPropertiesTable(mptWorld);

}

MyDetectorConstruction::~MyDetectorConstruction()
{}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{
    G4double x_world = 0.5*m, y_world = 0.5*m, z_world = 0.5*m;

    

    // 辐射介质和world几何设置
    solidWorld = new G4Box("solidWorld", x_world, y_world, z_world); // solid world，一个1m*1m*1m的立方体（-0.5～0.5m）
    logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld"); // logical world
    physWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true); // physical world G4ThreeVector是中心点坐标

    solidVacuum = new G4Box("vacuumWorld", 0.25*m, 0.25*m, 0.25*m);
    logicVacuum = new G4LogicalVolume(solidVacuum, vacuum, "logicVacuum");
    physVacuum = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicVacuum, "physVacuum", logicWorld, false, 0, true);

    //solidRadiator = new G4Box("solidRadiator", 0.5*m, 0.5*m, 0.1*m); // solid radiator
    //logicRadiator = new G4LogicalVolume(solidRadiator, Air_0, "logicRadiator"); // logical  radiator
    //physRadiator = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.38*m), logicRadiator, "physRadiator", logicWorld, false, 0, true); //  physical radiator

    //solidScintillator = new G4Tubs("solidScintillator", 0.*cm, 8.*cm, 10*cm, 0*deg, 360*deg);
    //logicScintillator = new G4LogicalVolume(solidScintillator, NaI, "logicScintillator");
    //physScintillator = new G4PVPlacement(0, G4ThreeVector(0., 0., 13.*cm), logicScintillator, "physScintillator", logicVacuum, false, 0, true);
    
    
    //solidHPGe = new G4Tubs("solidHPGe", 0.*cm, 5.*cm, 10*cm, 0*deg, 360*deg);
    //logicHPGe = new G4LogicalVolume(solidHPGe, HPGe, "logicHPGe");
    //physHPGe = new G4PVPlacement(0, G4ThreeVector(0., 0., 15.*cm), logicHPGe, "physHPGe", logicWorld, false, 0, true);
        

    solidPIPS = new G4Tubs("solidPIPS", 0.*cm, 2.5*cm, 0.02*cm, 0*deg, 360*deg);
    logicPIPS = new G4LogicalVolume(solidPIPS, pips, "logicPIPS");
    physPIPS = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.2*cm), logicPIPS, "physPIPS", logicVacuum, false, 0, true);

    fScoringVolume = logicPIPS;
    // 探测器构建
    // 先定义solid空间，由于之后定义的灵敏体积要外部访问探测器的logical空间，因此需要在头文件中MyDetectorConstruction类中定义
    // 探测器的logical空间
    solidDetector = new G4Box("solidDetector", x_world/nRows, y_world/nCols, 0.01*m); // solid detector
    logicDetector = new G4LogicalVolume(solidDetector, worldMat, "logicDetector");

    //fScoringVolume = logicScintillator;
    // 探测器阵列构建，使用for循环，构建一个100x100的探测器阵列，并给每一个探测器单元编号
    for(G4int i = 0; i < nRows; i++)
    {
        for(G4int j = 0; j < nCols; j++)
        {
            physDetector = new G4PVPlacement(0, G4ThreeVector(-(x_world)+(x_world/nRows)*(2*i+1), -(y_world)+(y_world/nCols)*(2*j+1), 0.49*m), logicDetector, "physDetector", logicWorld, false,  j+i*nRows, false);
        }
    }

    return physWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
    MySensitiveDetector *sensDet = new MySensitiveDetector("sensitiveDetector");
    logicDetector->SetSensitiveDetector(sensDet);
}
