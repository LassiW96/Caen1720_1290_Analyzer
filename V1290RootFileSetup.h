#ifndef V1290ROOTFILESETUP_H
#define V1290ROOTFILESETUP_H

#include <RtypesCore.h>
#include <TFile.h>
#include <TTree.h>

struct V1290EventData {
  Int_t eventNumber = 0;
  Float_t delta_t_ch0_ch1 = 0; // (ch0 - ch1) in ns
  Float_t delta_t_ch0_ch2 = 0; // (ch0 - ch2) in ns

  void clear() {
    eventNumber = 0;
    delta_t_ch0_ch1 = 0;
    delta_t_ch0_ch2 = 0;
  }
};

class V1290RootFileSetup {
public:
  V1290RootFileSetup(const std::string &outputFileName,
                     const std::string &treeName);
  ~V1290RootFileSetup();

  void setupBranches();

  // Members to hold branch data
  TFile *m_file = nullptr;
  TTree *m_tree = nullptr;

  V1290EventData m_event;

private:
  std::string m_outputFileName;
  std::string m_treeName;
};

#endif // V1290ROOTFILESETUP_H
