#ifndef V1720ROOTFILESETUP_H
#define V1720ROOTFILESETUP_H

#include <RtypesCore.h>
#include <TFile.h>
#include <TTree.h>

struct V1720ChannelData {
    std::vector<Float_t> samples;

    Float_t baseline = 0;
    Float_t integral = 0;
    Float_t peak     = 0;
    Float_t peakIdx  = 0;

    void resize(Int_t nSamples) {
        samples.resize(nSamples);
    }

    void clear() {
        std::fill(samples.begin(), samples.end(), 0);
        baseline = 0;
        integral = 0;
        peak     = 0;
        peakIdx  = 0;
    }
};

struct V1720EventData {
    Int_t eventNumber = 0;
    Float_t triggerTime = 0;

    std::vector<V1720ChannelData> channels;

    void resize(Int_t nChannels, Int_t nSamples) {
        channels.resize(nChannels);
        for (auto& ch : channels)
            ch.resize(nSamples);
    }

    void clear() {
        for (auto& ch : channels)
            ch.clear();
    }
};

class V1720RootFileSetup {
public:
    V1720RootFileSetup(const std::string& outputFileName, 
            const std::string& treeName,
            const Int_t& numChannels,
            const Int_t& numSamples);
    ~V1720RootFileSetup();

    void setupBranches();

    // Members to hold branch data
    TFile* m_file = nullptr;
    TTree* m_tree = nullptr;

    std::vector<Float_t> m_ch1;
    std::vector<Float_t> m_ch2;

    // For future expansion, we can also hold event-level data per channel here
    V1720EventData m_event;
    Int_t m_numChannels;
    Int_t m_numSamples;


private:
    std::string m_outputFileName;
    std::string m_treeName;
};

#endif // V1720ROOTFILESETUP_H