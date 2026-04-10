#ifndef TIROOTFILESETUP_H
#define TIROOTFILESETUP_H

#include <Rtypes.h>
#include <RtypesCore.h>
#include <TFile.h>
#include <TTree.h>
#include <string>
#include <vector>

// ---- V1720 ADC channel data ----
struct TIChannelData {
  std::vector<Float_t> samples;

  Float_t baseline = 0;
  Float_t integral = 0;
  Float_t peak = 0;
  Float_t peakIdx = 0;

  void resize(Int_t nSamples) { samples.resize(nSamples); }

  void clear() {
    std::fill(samples.begin(), samples.end(), 0.0f);
    baseline = 0;
    integral = 0;
    peak = 0;
    peakIdx = 0;
  }
};

// ---- Combined event data (ADC + TDC) ----
struct TIEventData {
  // --- Trigger / event-level ---
  Int_t eventNumber = 0;
  Float_t triggerTime = 0;

  // --- V1720 ADC ---
  std::vector<TIChannelData> channels; // one entry per ADC channel

  // --- V1290 TDC ---
  // refChannel: the lowest-numbered channel that fired (delta-t reference)
  Int_t refChannel = -1;
  Int_t refRawTime = -1;
  std::vector<Int_t> hit_channels; // channel numbers of all OTHER hits
  std::vector<Float_t> delta_t;    // delta-t (ns) relative to refChannel

  void resize(Int_t nChannels, Int_t nSamples) {
    channels.resize(nChannels);
    for (auto &ch : channels)
      ch.resize(nSamples);
  }

  void clear() {
    for (auto &ch : channels)
      ch.clear();
    triggerTime = 0;
    refChannel = -1;
    refRawTime = -1;
    hit_channels.clear();
    delta_t.clear();
  }
};

// ---- Root file / tree manager ----
class TIRootFileSetup {
public:
  TIRootFileSetup(const std::string &outputFileName,
                  const std::string &treeName, Int_t numChannels,
                  Int_t numSamples);
  ~TIRootFileSetup();

  void setupBranches();

  // Public so the decoder can access them directly
  TFile *m_file = nullptr;
  TTree *m_tree = nullptr;
  TIEventData m_event;

  Int_t m_numChannels;
  Int_t m_numSamples;

private:
  std::string m_outputFileName;
  std::string m_treeName;
};

#endif // TIROOTFILESETUP_H
