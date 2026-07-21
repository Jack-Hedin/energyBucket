#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>


// I copied this from the rtrk one, I dont know which of these I actually need

#include <fun4all/Fun4AllUtils.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllOutputManager.h>

#include <mbd/MbdReco.h>
#include <globalvertex/GlobalVertexReco.h>
#include <phool/recoConsts.h> //to get reco constants
#include <Calo_Calib.C> //for calibrations
#include <ffamodules/CDBInterface.h>

#include <string>
#include <fstream>
#include <iostream>




#include <TSystem.h>

// My two modules
#include <energybucket/energyBucket.h>
R__LOAD_LIBRARY(libenergyBucket.so)

R__LOAD_LIBRARY(libfun4all.so)

//I also copied this 


R__LOAD_LIBRARY(libjetbase.so)
R__LOAD_LIBRARY(libjetbackground.so)
R__LOAD_LIBRARY(libcalo_reco.so)
R__LOAD_LIBRARY(libffamodules.so)

R__LOAD_LIBRARY(libg4centrality.so)
R__LOAD_LIBRARY(libg4dst.so)
R__LOAD_LIBRARY(libjetbase.so)
R__LOAD_LIBRARY(libg4jets.so)
R__LOAD_LIBRARY(libtrack_reco.so)





void Fun4All_energyBucket(const int stop = 1, //when to stop the runs
	        	  const int should_exit = 1, //whether or not to exit root afterwords, 0 to stay
			  const int is_simfile = false, //If you are using a simulation file like pythia or something, set to 1
			  const string &inputListFile0 = "dst_calofitting.list",  //this is the location of the list I want to use
			  const string dbtag = "ProdA_2024" //Should be in the file names
			  ) {

  //assumably have to start the F4A server before actually doing anything
  Fun4AllServer *se = Fun4AllServer::instance();
  int verbosity = 0;
   
  if (!is_simfile){

  //get runnumber of first file
  ifstream file(inputListFile0);
  string first_file;
  getline(file, first_file);

  pair<int, int> runseg = Fun4AllUtils::GetRunSegment(first_file);
  int runnumber = runseg.first;
  cout << "run number = " << runnumber << endl;


  //set up all our reco constants
  recoConsts *rc = recoConsts::instance();
  rc->set_StringFlag("CDB_GLOBALTAG",dbtag);
  rc->set_uint64Flag("TIMESTAMP",runnumber);
  CDBInterface::instance() -> Verbosity(1);
  
  }


  //register the input, this seems to be the standard way to do thsi
  Fun4AllInputManager *in0 = new Fun4AllDstInputManager("in0");
  in0->AddListFile(inputListFile0);
  se->registerInputManager(in0);


  if (!is_simfile) {  
  //Can it wait for a bit? Im in the middle of some calibrations.
  // - Garrus Vakarian
  Process_Calo_Calib();

  //MBD Reconstruction for vertex stuff ig
  MbdReco *mbdreco = new MbdReco();
  se->registerSubsystem(mbdreco);

  GlobalVertexReco *gvertex = new GlobalVertexReco();
  gvertex->Verbosity(Fun4AllBase::VERBOSITY_QUIET);
  se->registerSubsystem(gvertex);
  }


  energyBucket *myEB = new energyBucket("energyBucket.root");
  se->registerSubsystem(myEB);

  //Run some number of events, 0 to run all in the list
  se->run(stop);
  se->End();

  //exits if you want it to, defaults to yes
  if (should_exit == 1) {
gSystem->Exit(0);
  }
}
