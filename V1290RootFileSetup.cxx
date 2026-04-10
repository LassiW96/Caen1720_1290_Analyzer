#include "V1290RootFileSetup.h"

V1290RootFileSetup::V1290RootFileSetup(const std::string &outputFileName,
                                       const std::string &treeName)
    : m_outputFileName(outputFileName), m_treeName(treeName) {
  m_file = new TFile(m_outputFileName.c_str(), "recreate");
  m_tree = new TTree(m_treeName.c_str(), "V1290A TDC delta-t tree");
}

void V1290RootFileSetup::setupBranches() {
  m_tree->Branch("eventNumber", &m_event.eventNumber);
  m_tree->Branch("hit_channels", &m_event.hit_channels);
  m_tree->Branch("delta_t", &m_event.delta_t);
}

V1290RootFileSetup::~V1290RootFileSetup() = default;
