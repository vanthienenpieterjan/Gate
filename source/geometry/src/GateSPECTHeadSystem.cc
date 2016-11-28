/*----------------------
  Copyright (C): OpenGATE Collaboration

  This software is distributed under the terms
  of the GNU Lesser General  Public Licence (LGPL)
  See GATE/LICENSE.txt for further details
  ----------------------*/

#include "GateSPECTHeadSystem.hh"

#include "G4UnitsTable.hh"

#include "GateClockDependentMessenger.hh"
#include "GateOutputMgr.hh"
#include "GateToProjectionSet.hh"
#include "GateToInterfile.hh"
#include "GateToGPUImageSPECT.hh"
#include "GateARFSD.hh"
#include "GateDetectorConstruction.hh"

// Constructor
GateSPECTHeadSystem::GateSPECTHeadSystem(const G4String& itsName)
  : GateVSystem( itsName , false ),
    m_gateToProjectionSet(0),
    m_gateToInterfile(0)
{
  m_ARFStage = -2;
  GateDetectorConstruction::GetGateDetectorConstruction()->insertARFSD(GetObjectName(),m_ARFStage);

  // Setup a messenger
  m_messenger = new GateClockDependentMessenger(this);
  m_messenger->SetDirectoryGuidance(G4String("Controls the system '") + GetObjectName() + "'" );

  m_messenger->SetARFCommands();

  // Define the scanner components
  m_crystalComponent = new GateSystemComponent("crystal",GetBaseComponent(),this);
  m_pixelComponent = new GateSystemComponent("pixel",m_crystalComponent,this);

  // Insert a projection-set maker and a Interfile writer into the output manager
  GateOutputMgr *outputMgr = GateOutputMgr::GetInstance();
  m_gateToProjectionSet = new GateToProjectionSet("projection", outputMgr,this,GateOutputMgr::GetDigiMode());
  outputMgr->AddOutputModule((GateVOutputModule*)m_gateToProjectionSet);
  m_gateToInterfile = new GateToInterfile("interfile", outputMgr,this,GateOutputMgr::GetDigiMode());
  outputMgr->AddOutputModule((GateVOutputModule*)m_gateToInterfile);

	// GPU output
	m_gateToGPUImageSPECT = new GateToGPUImageSPECT( "spectGPU",
                                                   outputMgr, this, GateOutputMgr::GetDigiMode() );
	outputMgr->AddOutputModule((GateVOutputModule*)m_gateToGPUImageSPECT);

  SetOutputIDName((char *)"headID",0);
  SetOutputIDName((char *)"crystalID",1);
  SetOutputIDName((char *)"pixelID",2);
}


// Destructor
GateSPECTHeadSystem::~GateSPECTHeadSystem()
{
  delete m_messenger;

  // Note that we don't delete our output modulesor components, as this will be done by the
  // output manager or byt the base-class respectively
}


void GateSPECTHeadSystem::setARFStage(G4String thestage)
{

  if ( thestage == "generateData" ) m_ARFStage = 0;
  else if ( thestage == "computeTables" ) m_ARFStage = 1;
  else if ( thestage == "useTables" )  m_ARFStage = 2;

  GateARFSD* ARFSD = GateDetectorConstruction::GetGateDetectorConstruction()->GetARFSD();

  if ( ARFSD != 0 ) ARFSD->SetStage( m_ARFStage );

  GateMessage("Geometry", 2, "GateSPECTHeadSystem::setARFStage stage = "
              << thestage << "  " << m_ARFStage);

  if ( m_ARFStage == -2 ) {
    G4String msg = "GateSPECTHeadSystem::setARFStage stage '"+thestage+"' is not valid.";
    G4Exception( "GateSPECTHeadSystem::setARFStage", "setARFStage", FatalException, msg);
  }
}


