//____________________________________________________________________________..
//
// This is a template for a Fun4All SubsysReco module with all methods from the
// $OFFLINE_MAIN/include/fun4all/SubsysReco.h baseclass
// You do not have to implement all of them, you can just remove unused methods
// here and in energyBucket.h.
//
// energyBucket(const std::string &name = "energyBucket")
// everything is keyed to energyBucket, duplicate names do work but it makes
// e.g. finding culprits in logs difficult or getting a pointer to the module
// from the command line
//
// energyBucket::~energyBucket()
// this is called when the Fun4AllServer is deleted at the end of running. Be
// mindful what you delete - you do loose ownership of object you put on the node tree
//
// int energyBucket::Init(PHCompositeNode *topNode)
// This method is called when the module is registered with the Fun4AllServer. You
// can create historgrams here or put objects on the node tree but be aware that
// modules which haven't been registered yet did not put antyhing on the node tree
//
// int energyBucket::InitRun(PHCompositeNode *topNode)
// This method is called when the first event is read (or generated). At
// this point the run number is known (which is mainly interesting for raw data
// processing). Also all objects are on the node tree in case your module's action
// depends on what else is around. Last chance to put nodes under the DST Node
// We mix events during readback if branches are added after the first event
//
// int energyBucket::process_event(PHCompositeNode *topNode)
// called for every event. Return codes trigger actions, you find them in
// $OFFLINE_MAIN/include/fun4all/Fun4AllReturnCodes.h
//   everything is good:
//     return Fun4AllReturnCodes::EVENT_OK
//   abort event reconstruction, clear everything and process next event:
//     return Fun4AllReturnCodes::ABORT_EVENT; 
//   proceed but do not save this event in output (needs output manager setting):
//     return Fun4AllReturnCodes::DISCARD_EVENT; 
//   abort processing:
//     return Fun4AllReturnCodes::ABORT_RUN
// all other integers will lead to an error and abort of processing
//
// int energyBucket::ResetEvent(PHCompositeNode *topNode)
// If you have internal data structures (arrays, stl containers) which needs clearing
// after each event, this is the place to do that. The nodes under the DST node are cleared
// by the framework
//
// int energyBucket::EndRun(const int runnumber)
// This method is called at the end of a run when an event from a new run is
// encountered. Useful when analyzing multiple runs (raw data). Also called at
// the end of processing (before the End() method)
//
// int energyBucket::End(PHCompositeNode *topNode)
// This is called at the end of processing. It needs to be called by the macro
// by Fun4AllServer::End(), so do not forget this in your macro
//
// int energyBucket::Reset(PHCompositeNode *topNode)
// not really used - it is called before the dtor is called
//
// void energyBucket::Print(const std::string &what) const
// Called from the command line - useful to print information when you need it
//
// [[maybe_unused]] suppresses compiler warnings if topNode is not used in this method
//
//____________________________________________________________________________..

#include "energyBucket.h"

#include <fun4all/Fun4AllReturnCodes.h>

#include <phool/PHCompositeNode.h>

//all of these are my own additions
#include <calobase/TowerInfoContainer.h>
#include <calobase/TowerInfo.h>
#include <calobase/RawTowerGeomContainer.h>
#include <phool/getClass.h>

//histograms/graphs
#include <TProfile2D.h>
#include <TFile.h>

//just to cout
#include <iostream>

//____________________________________________________________________________..
energyBucket::energyBucket(const std::string &name):
 SubsysReco("Energy_Buckets")
 , Outfile(name)
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

  //in large part taken from the tutorials/CaloDataRun24pp/ ana files
  delete out; 
  out = new TFile(Outfile.c_str(), "RECREATE");


  //For each graph stated here, it must also be stated in the header file to carry over
  //name, title, Nbins, min, max, (x3)
  h_OHCalE = new TProfile2D("OHCalE", "Measured Energy in OHCal;eta;phi;E (GeV)", 24, -1.1, 1.1, 64, 0, 6.3, -10, 90); 
  
  h_IHCalE = new TProfile2D("INCalE", "Measured Energy in INCal;eta;phi;E (GeV)", 24, -1.1, 1.1, 64, 0, 6.3, -10, 90);



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
  
  //Grab both outer and Inner HCAL nodes
  TowerInfoContainer *towersOHC = findNode::getClass<TowerInfoContainer>(topNode, "TOWERS_HCALOUT");
  TowerInfoContainer *towersIHC = findNode::getClass<TowerInfoContainer>(topNode, "TOWERS_HCALIN");  
  
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
  out->cd();
  
  
  h_OHCalE->Write();
  h_IHCalE->Write();  
  
  out->Close();
  delete out;


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
