//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: DetectorConstruction.cc 77656 2013-11-27 08:52:57Z gcosmo $
//
/// \file DetectorConstruction.cc
/// \brief Implementation of the DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4MaterialTable.hh"
#include "G4NistManager.hh"

#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVParameterised.hh"
#include "G4PVReplica.hh"
#include "G4UserLimits.hh"

#include "G4RunManager.hh"
#include "G4GenericMessenger.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4ios.hh"
#include "G4SystemOfUnits.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction(), 
  fMessenger(0),
  fHodoscope1Logical(0),
  fWirePlane1Logical(0),
  fVisAttributes(),
  fArmAngle(0.*deg), fArmRotation(0), fSecondArmPhys(0)

{
    fArmRotation = new G4RotationMatrix();
    fArmRotation->rotateY(fArmAngle);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
    delete fArmRotation;
    delete fMessenger;
    
    for (G4int i=0; i<G4int(fVisAttributes.size()); ++i) 
    {
      delete fVisAttributes[i];
    }  
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    // Construct materials
    ConstructMaterials();
    G4Material* air = G4Material::GetMaterial("G4_AIR");
    G4Material* argonGas = G4Material::GetMaterial("G4_Ar");
    G4Material* scintillator 
      = G4Material::GetMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    G4bool checkOverlaps = true;

    
    // geometries --------------------------------------------------------------
    // experimental hall (world volume)
    G4VSolid* worldSolid 
      = new G4Box("worldBox",10.*m,3.*m,10.*m);
    G4LogicalVolume* worldLogical
      = new G4LogicalVolume(worldSolid,air,"worldLogical");
    G4VPhysicalVolume* worldPhysical
      = new G4PVPlacement(0,G4ThreeVector(),worldLogical,"worldPhysical",0,
                          false,0,checkOverlaps);
    
    // first arm
    G4VSolid* firstArmSolid 
      = new G4Box("firstArmBox",1.5*m,1.*m,3.*m);
    G4LogicalVolume* firstArmLogical
      = new G4LogicalVolume(firstArmSolid,air,"firstArmLogical");
    new G4PVPlacement(0,G4ThreeVector(0.,0.,-5.*m),firstArmLogical,
                      "firstArmPhysical",worldLogical,
                      false,0,checkOverlaps);
    
    // second arm
    G4VSolid* secondArmSolid 
      = new G4Box("secondArmBox",2.*m,2.*m,3.5*m);
    G4LogicalVolume* secondArmLogical
      = new G4LogicalVolume(secondArmSolid,air,"secondArmLogical");
    G4double x = -5.*m * std::sin(fArmAngle);
    G4double z = 5.*m * std::cos(fArmAngle);
    fSecondArmPhys
      = new G4PVPlacement(fArmRotation,G4ThreeVector(x,0.,z),secondArmLogical,
                          "fSecondArmPhys",worldLogical,
                          false,0,checkOverlaps);
    
    
    // hodoscopes in first arm
    G4VSolid* hodoscope1Solid 
      = new G4Box("hodoscope1Box",5.*cm,20.*cm,0.5*cm);
    fHodoscope1Logical
      = new G4LogicalVolume(hodoscope1Solid,scintillator,"hodoscope1Logical");
    for (G4int i=0;i<15;i++)
    {
        G4double x1 = (i-7)*10.*cm;
        new G4PVPlacement(0,G4ThreeVector(x1,0.,-1.5*m),fHodoscope1Logical,
                          "hodoscope1Physical",firstArmLogical,
                          false,i,checkOverlaps);
    }
    
    // drift chambers in first arm
    G4VSolid* chamber1Solid 
      = new G4Box("chamber1Box",1.*m,30.*cm,1.*cm);
    G4LogicalVolume* chamber1Logical
      = new G4LogicalVolume(chamber1Solid,argonGas,"chamber1Logical");
    for (G4int i=0;i<5;i++)
    {
        G4double z1 = (i-2)*0.5*m;
        new G4PVPlacement(0,G4ThreeVector(0.,0.,z1),chamber1Logical,
                          "chamber1Physical",firstArmLogical,
                          false,i,checkOverlaps);
    }
    
    // "virtual" wire plane, "virtual" because it is a single volume
    // without the description of the wires
    G4VSolid* wirePlane1Solid 
      = new G4Box("wirePlane1Box",1.*m,30.*cm,0.1*mm);
    fWirePlane1Logical
      = new G4LogicalVolume(wirePlane1Solid,argonGas,"wirePlane1Logical");
    new G4PVPlacement(0,G4ThreeVector(0.,0.,0.),fWirePlane1Logical,
                      "wirePlane1Physical",chamber1Logical,
                      false,0,checkOverlaps);
    
    
    // =============================================
    // Exercise 2a,b
    // Create a box of CsI, Pb or Scintillator
    // (for nist materials remember to use NIST name, e.g. G4_Pb)
    // to absorb particles.
    // Observe how different absorber influence
    //  shower dimentions.
    // Some parameters:
    //   Dimensions (x,y,z): 300x60x100 cm
    //   Position: on the far back of the "second arm" volume
    // =============================================

    
    // visualization attributes ------------------------------------------------
    
    G4VisAttributes* visAttributes = new G4VisAttributes(G4Colour(1.0,1.0,1.0));
    visAttributes->SetVisibility(false);
    worldLogical->SetVisAttributes(visAttributes);
    fVisAttributes.push_back(visAttributes);
    
    visAttributes = new G4VisAttributes(G4Colour(1.0,1.0,1.0));
    visAttributes->SetVisibility(false);
    firstArmLogical->SetVisAttributes(visAttributes);
    secondArmLogical->SetVisAttributes(visAttributes);
    fVisAttributes.push_back(visAttributes);
    
    visAttributes = new G4VisAttributes(G4Colour(0.8888,0.0,0.0));
    fHodoscope1Logical->SetVisAttributes(visAttributes);
    fVisAttributes.push_back(visAttributes);
    
    visAttributes = new G4VisAttributes(G4Colour(0.0,1.0,0.0));
    chamber1Logical->SetVisAttributes(visAttributes);
    fVisAttributes.push_back(visAttributes);
    
    visAttributes = new G4VisAttributes(G4Colour(0.0,0.8888,0.0));
    visAttributes->SetVisibility(false);
    fWirePlane1Logical->SetVisAttributes(visAttributes);
    fVisAttributes.push_back(visAttributes);
    
    visAttributes = new G4VisAttributes(G4Colour(0.8888,0.8888,0.0));
    visAttributes->SetVisibility(false);
    fVisAttributes.push_back(visAttributes);
    
    // return the world physical volume ----------------------------------------
    
    return worldPhysical;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructSDandField()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructMaterials()
{
    G4NistManager* nistManager = G4NistManager::Instance();

    // Air 
    nistManager->FindOrBuildMaterial("G4_AIR");
  
    // Argon gas
    nistManager->FindOrBuildMaterial("G4_Ar");

    // Scintillator
    // (PolyVinylToluene, C_9H_10)
    nistManager->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    
    // CsI
    // =============================================
    // Exercise 1a
    //    Create material Cesium Iodide.
    //  Chemical formula CsI.
    // Some properties:
    //    Cs : A=55 Zeff=132.9*g/mol
    //     I : A=53 , Zeff=126.9*g/mol
    // Density of christal of CsI is rho=4.51*g/cm^3
    // =============================================
    
    // Lead
    // =============================================
    // Exercise 1b
    //    Create material Lead from Nist Manager.
    // Note that it is actually a mixture of several isotopes.
    // If you want to build it by hand starting from isotopes you
    // can:
    //    G4Isotope* iso1= new G4Isotope( ... )
    //    ....
    //    G4Element* elem = new G4Element("name","symbol",nisotopes);
    //    elem->AddIsotope( iso1 );
    //    elem->AddIsotope( iso2 );
    //    ...
    // =============================================
    
    
    // Important: Never use a real gas with 0 density as materials.
    //            Physics processes requires density>0 and may give wrong
    //            results if they encounter 0 density material.
    //            Instead use one of the following methods if you need
    //            "vacuum" (e.g. a beam pipe internal)
    // Vacuum "Galactic"
    // nistManager->FindOrBuildMaterial("G4_Galactic");

    // Vacuum "Air with low density"
    // G4Material* air = G4Material::GetMaterial("G4_AIR");
    // G4double density = 1.0e-5*air->GetDensity();
    // nistManager
    //   ->BuildMaterialWithNewDensity("Air_lowDensity", "G4_AIR", density);

    G4cout << G4endl << "The materials defined are : " << G4endl << G4endl;
    G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
