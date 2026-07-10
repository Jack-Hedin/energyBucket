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





void Fun4All_energyBucket(int should_exit = 1) {
  //this is the location of the list I want to use
  const string &inputFile1 = "dst_calo_cluster.list";


  //assumably have to start the F4A server before actually doing anything
  Fun4AllServer *se = Fun4AllServer::instance();
  int verbosity = 0;

  //register the input, this seems to be the standard way to do thsi
  Fun4AllInputManager *in0 = new Fun4AllDstInputManager("in0");
  in0->AddListFile(inputFile1);
  se->registerInputManager(in0);


  energyBucket *myEB = new energyBucket("energyBucket.root");
  se->registerSubsystem(myEB);

  //Run some number of events, 0 to run all in the list
  se->run(1);
  se->End();

  //exits if you want it to, defaults to yes
  if (should_exit == 1) {
gSystem->Exit(0);
  }
}
