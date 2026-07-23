// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef ENERGYBUCKET_H
#define ENERGYBUCKET_H

#include <fun4all/SubsysReco.h>
#include <string>
#include <cstring>

class PHCompositeNode;
class TProfile2D;
class TFile;


class energyBucket : public SubsysReco
{
 public:

  energyBucket(const std::string &name = "energyBucket");

  ~energyBucket() override;

  /** Called during initialization.
      Typically this is where you can book histograms, and e.g.
      register them to Fun4AllServer (so they can be output to file
      using Fun4AllServer::dumpHistos() method).
   */
  int Init(PHCompositeNode *topNode) override;

  /** Called for first event when run number is known.
      Typically this is where you may want to fetch data from
      database, because you know the run number. A place
      to book histograms which have to know the run number.
   */
  int InitRun(PHCompositeNode *topNode) override;

  /** Called for each event.
      This is where you do the real work.
   */
  int process_event(PHCompositeNode *topNode) override;

  /// Clean up internals after each event.
  int ResetEvent(PHCompositeNode *topNode) override;

  /// Called at the end of each run.
  int EndRun(const int runnumber) override;

  /// Called at the end of all processing.
  int End(PHCompositeNode *topNode) override;

  /// Reset
  int Reset(PHCompositeNode * /*topNode*/) override;

  void set_wait(int wait) {m_wait = wait;}

 private:

 
  TProfile2D *h_OHCalE = nullptr;
  TProfile2D *h_OHCalE_calib = nullptr;

  TProfile2D *h_IHCalE = nullptr;
  TProfile2D *h_IHCalE_calib = nullptr;
  
  TProfile2D *h_CEMCE = nullptr;
  TProfile2D *h_CEMCE_calib = nullptr;

  std::string m_outfile{"commissioning.root"};
  int m_wait;
  int m_eventNumber;
};

#endif // ENERGYBUCKET_H
