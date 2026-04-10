#include "TIRootFileSetup.h"

TIRootFileSetup::TIRootFileSetup(const std::string &outputFileName,
                                 const std::string &treeName, Int_t numChannels,
                                 Int_t numSamples)
    : m_outputFileName(outputFileName), m_treeName(treeName),
      m_numChannels(numChannels), m_numSamples(numSamples) {
  m_file = new TFile(m_outputFileName.c_str(), "recreate");
  m_tree = new TTree(m_treeName.c_str(), "TI combined ADC+TDC tree");

  m_event.resize(m_numChannels, m_numSamples);
}

void TIRootFileSetup::setupBranches() {
  // ---- Event-level ----
  m_tree->Branch("eventNumber", &m_event.eventNumber);
  m_tree->Branch("triggerTime", &m_event.triggerTime);

  // ---- V1720 ADC channel branches ----
  for (Int_t ch = 0; ch < m_numChannels; ch++) {
    std::string chName = "ch" + std::to_string(ch);

    m_tree->Branch((chName + "_samples").c_str(),
                   &m_event.channels[ch].samples);
    m_tree->Branch((chName + "_baseline").c_str(),
                   &m_event.channels[ch].baseline);
    m_tree->Branch((chName + "_integral").c_str(),
                   &m_event.channels[ch].integral);
    m_tree->Branch((chName + "_peak").c_str(), &m_event.channels[ch].peak);
    m_tree->Branch((chName + "_peakIdx").c_str(),
                   &m_event.channels[ch].peakIdx);
  }

  // ---- V1290 TDC branches ----
  // refChannel: lowest-numbered channel with a hit (delta-t reference)
  m_tree->Branch("tdc_refChannel", &m_event.refChannel);
  m_tree->Branch("tdc_refRawTime", &m_event.refRawTime);
  m_tree->Branch("tdc_hit_channels", &m_event.hit_channels);
  m_tree->Branch("tdc_delta_t", &m_event.delta_t);
}

TIRootFileSetup::~TIRootFileSetup() = default;
