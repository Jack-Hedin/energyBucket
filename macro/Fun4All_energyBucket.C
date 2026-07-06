#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>


#include <TSystem.h>

// #include <energyBucket/energyBucket.h>


// R__LOAD_LIBRARY(libenergyBucket.so)

R__LOAD_LIBRARY(libfun4all.so)


void Fun4All_energyBucket() {
  //this is the location of the list I want to use
  const string &inputFile1 = "dst_calo_cluster.list";


  //assumably have to start the F4A server before actually doing anything
  Fun4AllServer *se = Fun4AllServer::instance();
  int verbosity = 0;

  //register the input, this seems to be the standard way to do thsi
  Fun4AllInputManager *in0 = new Fun4AllDstInputManager("in0");
  in0->AddListFile(inputFile1);
  se->registerInputManager(in0);


  
  se->run(1);


  
  std::cout << "Test" << std::endl;
  
  
  se->End();
  gSystem->Exit(1);
}
