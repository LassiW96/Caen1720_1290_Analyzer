#include "V1720RootFileSetup.h"

V1720RootFileSetup::V1720RootFileSetup(const std::string& outputFileName,
        const std::string& treeName,
        const Int_t& numChannels,
        const Int_t& numSamples) : m_outputFileName(outputFileName), 
                                   m_treeName(treeName), 
                                   m_numChannels(numChannels), 
                                   m_numSamples(numSamples)
{
    m_file = new TFile(m_outputFileName.c_str(), "recreate");
    m_tree = new TTree(m_treeName.c_str(), "a simple Tree with simple variables");

    m_event.resize(m_numChannels, m_numSamples);
}

void V1720RootFileSetup::setupBranches() 
{
    // Event-level branches
    m_tree->Branch("eventNumber", &m_event.eventNumber);
    m_tree->Branch("triggerTime", &m_event.triggerTime);

    // Channel branches
    for (Int_t ch = 0; ch < m_numChannels; ch++) {

        std::string chName = "ch" + std::to_string(ch);

        m_tree->Branch((chName + "_samples").c_str(),
                       &m_event.channels[ch].samples);

        m_tree->Branch((chName + "_baseline").c_str(),
                       &m_event.channels[ch].baseline);

        m_tree->Branch((chName + "_integral").c_str(),
                       &m_event.channels[ch].integral);

        m_tree->Branch((chName + "_peak").c_str(),
                       &m_event.channels[ch].peak);

        m_tree->Branch((chName + "_peakIdx").c_str(),
                       &m_event.channels[ch].peakIdx);
    }
}


V1720RootFileSetup::~V1720RootFileSetup() = default;