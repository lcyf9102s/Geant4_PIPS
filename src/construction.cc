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
    Al_mat = nist->FindOrBuildMaterial("G4_Al");
    Am_mat = nist->FindOrBuildMaterial("G4_Fe");
    
    // Define Vacuum
    //vacuum = new G4Material("Vacuum", density= 1.e-5*g/cm3, ncomponents=1, kStateGas, 153*kelvin, 1.e-8*atmosphere);
    //vacuum->AddMaterial(Air_0, fractionmass=1.);
    vacuum = nist->FindOrBuildMaterial("G4_Galactic");


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

    // 1. 定义元素
    G4double a;  // 原子量
    G4double z;  // 原子序数

    G4Element* elFe = new G4Element("Iron", "Fe", z=26., a=55.845*g/mole);
    G4Element* elCr = new G4Element("Chromium", "Cr", z=24., a=51.9961*g/mole);
    G4Element* elNi = new G4Element("Nickel", "Ni", z=28., a=58.6934*g/mole);
    G4Element* elMo = new G4Element("Molybdenum", "Mo", z=42., a=95.95*g/mole);
    G4Element* elMn = new G4Element("Manganese", "Mn", z=25., a=54.938*g/mole);
    G4Element* elSi = new G4Element("Silicon", "Si", z=14., a=28.0855*g/mole);
    G4Element* elC  = new G4Element("Carbon", "C", z=6., a=12.0107*g/mole);
    G4Element* elS  = new G4Element("Sulfur", "S", z=16., a=32.065*g/mole);
    G4Element* elP  = new G4Element("Phosphorus", "P", z=15., a=30.9738*g/mole);
    G4Element* elN  = new G4Element("Nitrogen", "N", z=7., a=14.0067*g/mole);

    // 2. 创建材料
    G4double density = 8.0*g/cm3;
    steel316L = new G4Material("316L_Stainless_Steel", density, 10);

    steel316L->AddElement(elFe, 65.345*perCent);
    steel316L->AddElement(elCr, 17.0*perCent);
    steel316L->AddElement(elNi, 12.0*perCent);
    steel316L->AddElement(elMo, 2.5*perCent);
    steel316L->AddElement(elMn, 2.0*perCent);
    steel316L->AddElement(elSi, 1.0*perCent);
    steel316L->AddElement(elC, 0.03*perCent);
    steel316L->AddElement(elS, 0.03*perCent);
    steel316L->AddElement(elP, 0.045*perCent);
    steel316L->AddElement(elN, 0.05*perCent);


    

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

    // 定义元素
    G4Element* icruH = nist->FindOrBuildElement("H");
    G4Element* icruC = nist->FindOrBuildElement("C");
    G4Element* icruN = nist->FindOrBuildElement("N");
    G4Element* icruO = nist->FindOrBuildElement("O");

    // 定义 ICRU 球材料
    G4double ICRU_density = 1.0 * g/cm3;
    icruSphereMaterial = new G4Material("ICRUSphereMaterial", ICRU_density, 4);

    icruSphereMaterial->AddElement(icruH, 10.1 * perCent);
    icruSphereMaterial->AddElement(icruC, 11.1 * perCent);
    icruSphereMaterial->AddElement(icruN, 2.6 * perCent);
    icruSphereMaterial->AddElement(icruO, 76.2 * perCent);

}

MyDetectorConstruction::~MyDetectorConstruction()
{}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{
    G4double x_world = 2.*m, y_world = 2.*m, z_world = 2.*m;
    
    // 创建一个旋转矩阵，将圆柱体从 Z 轴对齐到 Y 轴
    G4RotationMatrix* rotation = new G4RotationMatrix();
    rotation->rotateX(90.*deg); // 绕 X 轴旋转 90 度
    // 辐射介质和world几何设置
    solidWorld = new G4Box("solidWorld", x_world, y_world, z_world); // solid world，一个1m*1m*1m的立方体（-0.5～0.5m）
    logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicWorld"); // logical world
    physWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, true); // physical world G4ThreeVector是中心点坐标

    solidVacuum = new G4Box("vacuumWorld", 1.6*m, 1.6*m, 1.6*m);
    logicVacuum = new G4LogicalVolume(solidVacuum, Air_0, "logicVacuum");
    physVacuum = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicVacuum, "physVacuum", logicWorld, false, 0, true);

    //solidwater = new G4Box("waterWorld", 1.6*m, 1.6*m, 0.5*m);
    //logicwater = new G4LogicalVolume(solidwater, H2O, "logicwater");
    //physwater = new G4PVPlacement(0, G4ThreeVector(0., 0., 1.*m), logicwater, "physwater", logicVacuum, false, 0, true);

    //solidContainer = new G4Tubs("solidContainer", 60.*cm, 61.*cm, 100*cm, 0*deg, 360*deg);
    //logicContainer = new G4LogicalVolume(solidContainer, steel316L, "logicContainer");
    //physContainer = new G4PVPlacement(rotation, G4ThreeVector(0., 0., 0.*cm), logicContainer, "physContainer", logicVacuum, false, 0, true);

    solidRadiator = new G4Box("solidRadiator", 0.1*m, 0.1*m, 0.1*m); // solid radiator
    logicRadiator = new G4LogicalVolume(solidRadiator, icruSphereMaterial, "logicRadiator"); // logical  radiator
    physRadiator = new G4PVPlacement(0, G4ThreeVector(0., 0., 1*m), logicRadiator, "physRadiator", logicVacuum, false, 0, true); //  physical radiator

    //solidScintillator = new G4Tubs("solidScintillator", 0.*cm, 8.*cm, 10*cm, 0*deg, 360*deg);
    //logicScintillator = new G4LogicalVolume(solidScintillator, NaI, "logicScintillator");
    //physScintillator = new G4PVPlacement(0, G4ThreeVector(0., 0., 100.*cm), logicScintillator, "physScintillator", logicVacuum, false, 0, true);
    
    
    //solidHPGe = new G4Tubs("solidHPGe", 0.*cm, 5.*cm, 10*cm, 0*deg, 360*deg);
    //logicHPGe = new G4LogicalVolume(solidHPGe, HPGe, "logicHPGe");
    //physHPGe = new G4PVPlacement(0, G4ThreeVector(0., 0., 15.*cm), logicHPGe, "physHPGe", logicWorld, false, 0, true);
    G4double pRmin = 0 * mm, pRmax = 20 * mm, pDz = 2 * mm;
    G4double pSphi = 0 * deg, pDphi = 360 * deg;
    solidAm = new G4Tubs("Am-241", pRmin, pRmax, 0.5 * pDz, pSphi, pDphi);
    //G4Box *solidAm = new G4Box("Am-241", 75 * mm, 75 * mm, 0.5 * pDz);

    
    logicAm = new G4LogicalVolume(solidAm,   //its solid
                            Am_mat,    //its material
                            "Am-241"); //its name

    G4ThreeVector pos1 = G4ThreeVector(0 * mm, 0 * mm, -2.5 * mm);
    physAm = new G4PVPlacement(0,              //no rotation
                      pos1,           //at position
                      logicAm,        //its logical volume
                      "Am-241",       //its name
                      logicVacuum,       //its mother  volume
                      false,          //no boolean operation
                      0,              //copy number
                      true); //overlaps checking
    
    G4double fpRmin = 8 * mm, fpRmax = 20 * mm, fpDz = 1.5 * mm;
    G4double fpSphi = 0 * deg, fpDphi = 360 * deg;
    solidfAm = new G4Tubs("fAm-241", fpRmin, fpRmax, 0.5 * fpDz, fpSphi, fpDphi);
    
    
    logicfAm = new G4LogicalVolume(solidfAm,   //its solid
                            Am_mat,     //its material
                            "fAm-241"); //its name

    G4ThreeVector fpos1 = G4ThreeVector(0 * mm, 0 * mm, -0.75 * mm);
    physfAm = new G4PVPlacement(0,              //no rotation
                      fpos1,          //at position
                      logicfAm,       //its logical volume
                      "fAm-241",      //its name
                      logicVacuum,       //its mother  volume
                      false,          //no boolean operation
                      0,              //copy number
                      true); //overlaps checking                  

    
    solidDeadLayer = new G4Tubs("solidDeadLayer", 0.*cm, 1*cm, 0.025 * um, 0*deg, 360*deg);
    logicDeadLayer = new G4LogicalVolume(solidDeadLayer, pips, "logicDeadLayer");
    physDeadLayer = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.025 * um), logicDeadLayer, "physDeadLayer", logicVacuum, false, 0, true);
    
    solidPIPS = new G4Tubs("solidPIPS", 0.*cm, 1*cm, 0.325 * mm, 0*deg, 360*deg);
    logicPIPS = new G4LogicalVolume(solidPIPS, pips, "logicPIPS");
    physPIPS = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.325 * mm + 0.05 * um), logicPIPS, "physPIPS", logicVacuum, false, 0, true);

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
            physDetector = new G4PVPlacement(0, G4ThreeVector(-(x_world)+(x_world/nRows)*(2*i+1), -(y_world)+(y_world/nCols)*(2*j+1), 1.99*m), logicDetector, "physDetector", logicWorld, false,  j+i*nRows, false);
        }
    }

    return physWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
    MySensitiveDetector *sensDet = new MySensitiveDetector("sensitiveDetector");
    logicDetector->SetSensitiveDetector(sensDet);
}
