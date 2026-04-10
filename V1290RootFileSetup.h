#ifndef V1290ROOTFILESETUP_H
#define V1290ROOTFILESETUP_H

#include <RtypesCore.h>
#include <TFile.h>
#include <TTree.h>

struct V1290EventData {
  uint32_t eventNumber = 0;
  std::vector<uint32_t> hit_channels;
  std::vector<Float_t> delta_t;

  void clear() {
    eventNumber = 0;
    hit_channels.clear();
    delta_t.clear();
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
