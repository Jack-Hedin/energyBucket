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
  h_OHCalE = new TProfile2D("Raw Outer HCalE", "Measured Energy in OHCal;eta;phi;E (ADS)", 24, -1.1, 1.1, 64, 0, 6.3, -10, 90); 
 
  h_OHCalE_calib = new TProfile2D("Calibrated Outer HCalE", "Measured Energy in OHCal;eta;phi;E (GeV)", 24, -1.1, 1.1, 64, 0, 6.3, -10, 90);



  h_IHCalE = new TProfile2D("IHCalE", "Measured Energy in Inner HCal;eta;phi;E (ADS)", 24, -1.1, 1.1, 64, 0, 6.3, -10, 90);

  h_IHCalE_calib = new TProfile2D("Calibrated Inner HCalE", "Measured Energy in IHCal;eta;phi;E (GeV)", 24, -1.1, 1.1, 64, 0, 6.3, -10, 90);

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
  

  TowerInfoContainer *towersOHC_C = findNode::getClass<TowerInfoContainer>(topNode, "TOWERINFO_CALIB_HCALOUT");
  TowerInfoContainer *towersIHC_C = findNode::getClass<TowerInfoContainer>(topNode, "TOWERINFO_CALIB_HCALIN");




  //Check if either was missing
  if (!towersOHC) {
    std::cout << "Hey! theres no HCALOUT towers here!" << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }  
  if (!towersIHC) {
    std::cout << "Hey! theres no HCALIN towers here!" << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  
  
  
  //Grab the Geometry data of the towers 
  RawTowerGeomContainer *geomOHC = findNode::getClass<RawTowerGeomContainer>(topNode, "TOWERGEOM_HCALOUT");
  RawTowerGeomContainer *geomIHC = findNode::getClass<RawTowerGeomContainer>(topNode, "TOWERGEOM_HCALIN");
  
  //Check if either was missing
  if (!geomOHC) {
    std::cout << "No OUTCAL Geometry???" << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  if (!geomIHC) {
    std::cout << "No INCAL Geometry???" << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  

  //loops over ever channel in the outer hcal. 
  for (unsigned int channel = 0; channel < towersOHC->size();channel++){
    
    //access the tower at each channel, the key lets us access the geometric data
    TowerInfo *leTower = towersOHC->get_tower_at_channel(channel);
    unsigned int towerKey = towersOHC->encode_key(channel);
  
    //graph her
    h_OHCalE->Fill(geomOHC->get_etacenter(towersOHC->getTowerEtaBin(towerKey)),
		   geomOHC->get_phicenter(towersOHC->getTowerPhiBin(towerKey)),
		   leTower->get_energy());
  }
  
    //Same thing but for the inner hcal. Technically could have had this be the same loop because its the name amount in both
    //but I really dont care, its neater this way
  for (unsigned int channel = 0; channel < towersIHC->size();channel++){

    //access the tower at each channel, the key lets us access the geometric data
    TowerInfo *leTower = towersIHC->get_tower_at_channel(channel);
    unsigned int towerKey = towersIHC->encode_key(channel);

    //graph her
    h_IHCalE->Fill(geomIHC->get_etacenter(towersIHC->getTowerEtaBin(towerKey)),
		   geomIHC->get_phicenter(towersIHC->getTowerPhiBin(towerKey)),
		   leTower->get_energy());
  }

  // ==========================================================================
  // Calibrated Tower Processing
  // ==========================================================================

  // Check if calibrated containers are missing
  if (!towersOHC_C) {
    std::cout << "Hey! theres no HCALOUT_CALIB towers here?" << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  if (!towersIHC_C) {
    std::cout << "Hey! theres no HCALOUT_CALIB towers here?" << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  // Loop over every channel in the calibrated outer hcal
  for (unsigned int channel = 0; channel < towersOHC_C->size(); channel++) {
    TowerInfo *leTowerCalib = towersOHC_C->get_tower_at_channel(channel);
    unsigned int towerKey = towersOHC_C->encode_key(channel);

    h_OHCalE_calib->Fill(geomOHC->get_etacenter(towersOHC_C->getTowerEtaBin(towerKey)),
                         geomOHC->get_phicenter(towersOHC_C->getTowerPhiBin(towerKey)),
                         leTowerCalib->get_energy());
  }

  // Loop over every channel in the calibrated inner hcal
  for (unsigned int channel = 0; channel < towersIHC_C->size(); channel++) {
    TowerInfo *leTowerCalib = towersIHC_C->get_tower_at_channel(channel);
    unsigned int towerKey = towersIHC_C->encode_key(channel);

    h_IHCalE_calib->Fill(geomIHC->get_etacenter(towersIHC_C->getTowerEtaBin(towerKey)),
                         geomIHC->get_phicenter(towersIHC_C->getTowerPhiBin(towerKey)),
                         leTowerCalib->get_energy());
  }  



  //legacy code to print it if I wanted to (to have inside the for loop)
  //std::cout << "The tower located at Phi = " << geomOHC->get_phicenter(towersOHC->getTowerPhiBin(towerKey))
  // 	      << " and at Eta = " << geomOHC->get_etacenter(towersOHC->getTowerEtaBin(towerKey)) 
  // 	      << ", has and energy of " << leTower->get_energy() << " GeV :3 :3 :3" << std::endl; 
  
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
