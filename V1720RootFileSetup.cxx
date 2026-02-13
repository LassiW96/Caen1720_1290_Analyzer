#include "V1720RootFileSetup.h"

V1720RootFileSetup::V1720RootFileSetup(const std::string& outputFileName, const std::string& treeName, const Int_t& numSamples) :
    m_file(nullptr), m_tree(nullptr), m_outputFileName(outputFileName), m_treeName(treeName), m_numSamples(numSamples)
{
    m_file = new TFile(m_outputFileName.c_str(), "recreate");
    m_tree = new TTree(m_treeName.c_str(), "a simple Tree with simple variables");
}

void V1720RootFileSetup::setupBranches(const std::string& branch1, const std::string& branch2) 
{
    // Setup branches here, e.g.:
    m_tree->Branch("numSamples", &m_numSamples, "numSamples/I");
    m_tree->Branch(branch1.c_str(), &m_ch1);
    m_tree->Branch(branch2.c_str(), &m_ch2);
}


V1720RootFileSetup::~V1720RootFileSetup() = default;