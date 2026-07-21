#include "energyBucket.h"

#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/PHCompositeNode.h>

//all of these are my own additions
#include <calobase/TowerInfoContainer.h>
#include <calobase/TowerInfo.h>
#include <calobase/RawTowerGeomContainer.h>
#include <phool/getClass.h>
#include <fun4all/PHTFileServer.h>

//histograms/graphs
#include <TProfile2D.h>
#include <TFile.h>

//just to cout
#include <iostream>

//____________________________________________________________________________..
energyBucket::energyBucket(const std::string &name):
 SubsysReco("Energy_Buckets")
 , m_outfile(name)
{
  std::cout << "energyBucket::energyBucket(const std::string &name) Calling ctor" << std::endl;
}

//____________________________________________________________________________..
energyBucket::~energyBucket()
{
  std::cout << "energyBucket::~energyBucket() Calling dtor" << std::endl;
}

//____________________________________________________________________________..
int energyBucket::Init([[maybe_unused]]PHCompositeNode *topNode)
{
  std::cout << "energyBucket::Init(PHCompositeNode *topNode) Initializing" << std::endl;

   
  PHTFileServer::get().open(m_outfile, "RECREATE");

  //For each graph stated here, it must also be stated in the header file to carry over
  //name, title, Nbins, min, max, (x3)
  h_OHCalE = new TProfile2D("Raw Outer HCalE", "Measured Energy in OHCal;eta;phi;E (ADS)", 24, -1.06, 1.06, 64, 0, 6.3, -10, 90);  
  h_OHCalE_calib = new TProfile2D("Calibrated Outer HCalE", "Measured Energy in OHCal;eta;phi;E (GeV)", 24, -1.06, 1.06, 64, 0, 6.3, -10, 90);

  h_IHCalE = new TProfile2D("IHCalE", "Measured Energy in Inner HCal;eta;phi;E (ADS)", 24, -1.06, 1.06, 64, 0, 6.3, -10, 90);
  h_IHCalE_calib = new TProfile2D("Calibrated Inner HCalE", "Measured Energy in IHCal;eta;phi;E (GeV)", 24, -1.06, 1.06, 64, 0, 6.3, -10, 90);

  h_CEMCE = new TProfile2D("CEMCE", "Measured Energy in EMCal;eta;phi;E (ADS)", 96, -1.111, 1.111, 256, -0.0737, 6.161, -10, 90);
  h_CEMCE_calib = new TProfile2D("Calibrated CEMCE", "Measured Energy in EMCal;eta;phi;E (GeV)", 96, -1.111, 1.111, 256, -0.0737, 6.161, -10, 90);


  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int energyBucket::InitRun([[maybe_unused]] PHCompositeNode *topNode)
{
  std::cout << "energyBucket::InitRun(PHCompositeNode *topNode) Initializing for Run XXX" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int energyBucket::process_event(PHCompositeNode *topNode)
{
  
  //Grabbing all the nodes we need
  TowerInfoContainer *towersOHC = findNode::getClass<TowerInfoContainer>(topNode, "TOWERS_HCALOUT");
  TowerInfoContainer *towersIHC = findNode::getClass<TowerInfoContainer>(topNode, "TOWERS_HCALIN");   
  TowerInfoContainer *towersCEMC = findNode::getClass<TowerInfoContainer>(topNode, "TOWERS_CEMC");
  

  TowerInfoContainer *towersOHC_C = findNode::getClass<TowerInfoContainer>(topNode, "TOWERINFO_CALIB_HCALOUT"); 
  TowerInfoContainer *towersIHC_C = findNode::getClass<TowerInfoContainer>(topNode, "TOWERINFO_CALIB_HCALIN");
  TowerInfoContainer *towersCEMC_C = findNode::getClass<TowerInfoContainer>(topNode, "TOWERINFO_CALIB_CEMC"); 
  

  RawTowerGeomContainer *geomOHC = findNode::getClass<RawTowerGeomContainer>(topNode, "TOWERGEOM_HCALOUT");
  RawTowerGeomContainer *geomIHC = findNode::getClass<RawTowerGeomContainer>(topNode, "TOWERGEOM_HCALIN");
  RawTowerGeomContainer *geomCEMC = findNode::getClass<RawTowerGeomContainer>(topNode, "TOWERGEOM_CEMC");


  // Comprehensive safety checks upfront
  if (!towersOHC)   { std::cout << "Missing HCALOUT towers!" << std::endl; return Fun4AllReturnCodes::ABORTRUN; }
  if (!towersIHC)   { std::cout << "Missing HCALIN towers!" << std::endl;  return Fun4AllReturnCodes::ABORTRUN; }
  if (!towersOHC_C) { std::cout << "Missing Calibrated HCALOUT towers!" << std::endl; return Fun4AllReturnCodes::ABORTRUN; }
  if (!towersIHC_C) { std::cout << "Missing Calibrated HCALIN towers!" << std::endl;  return Fun4AllReturnCodes::ABORTRUN; }
  if (!geomOHC)     { std::cout << "Missing OUTCAL Geometry!" << std::endl; return Fun4AllReturnCodes::ABORTRUN; }
  if (!geomIHC)     { std::cout << "Missing INCAL Geometry!" << std::endl;  return Fun4AllReturnCodes::ABORTRUN; }

  
  // Define a local structure to bundle the related objects together
  struct CaloLoopConfig {
    TowerInfoContainer* container;
    RawTowerGeomContainer* geometry;
    TProfile2D* histogram;
  };

  // yurt
  CaloLoopConfig configs[] = {
    {towersOHC,   geomOHC, h_OHCalE},
    {towersIHC,   geomIHC, h_IHCalE},
    {towersOHC_C, geomOHC, h_OHCalE_calib},
    {towersIHC_C, geomIHC, h_IHCalE_calib},
    {towersCEMC,  geomCEMC, h_CEMCE},
    {towersCEMC_C,geomCEMC, h_CEMCE_calib}
  };


  // Run everything through a single unified loop block
  for (const auto& config : configs) {
    
    //std::cout << "===========================================ΝEW TOWER SYSTEM===============================================" << std::endl;

    for (unsigned int channel = 0; channel < config.container->size(); channel++) {
      TowerInfo *leTower = config.container->get_tower_at_channel(channel);
      unsigned int towerKey = config.container->encode_key(channel);
          
      //std::cout << "Tower Eta: " << config.geometry->get_etacenter(config.container->getTowerEtaBin(towerKey)) << std::endl;
      //std::cout << "Tower Phi: " << config.geometry->get_phicenter(config.container->getTowerPhiBin(towerKey)) << std::endl;


      config.histogram->Fill(
        config.geometry->get_etacenter(config.container->getTowerEtaBin(towerKey)),
        config.geometry->get_phicenter(config.container->getTowerPhiBin(towerKey)),
        leTower->get_energy()
      );
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int energyBucket::ResetEvent([[maybe_unused]] PHCompositeNode *topNode)
{
  std::cout << "energyBucket::ResetEvent(PHCompositeNode *topNode) Resetting internal structures, prepare for next event" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int energyBucket::EndRun(const int runnumber)
{
  TFile fout(m_outfile.c_str(), "RECREATE");
  
  //****************
  h_OHCalE->Write();
  h_IHCalE->Write();  
  h_OHCalE_calib->Write();
  h_IHCalE_calib->Write();
  h_CEMCE->Write();
  h_CEMCE_calib->Write();
  //****************

  fout.Close();

  std::cout << "energyBucket::EndRun(const int runnumber) Ending Run for Run " << runnumber << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int energyBucket::End([[maybe_unused]] PHCompositeNode *topNode)
{
  std::cout << "energyBucket::End(PHCompositeNode *topNode) This is the End..." << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int energyBucket::Reset([[maybe_unused]] PHCompositeNode *topNode)
{
 std::cout << "energyBucket::Reset(PHCompositeNode *topNode) being Reset" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
void energyBucket::Print(const std::string &what) const
{
  std::cout << "energyBucket::Print(const std::string &what) const Printing info for " << what << std::endl;
}
