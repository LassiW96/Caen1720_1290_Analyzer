#ifndef V1720ROOTFILESETUP_H
#define V1720ROOTFILESETUP_H

#include <RtypesCore.h>
#include <TFile.h>
#include <TTree.h>

class V1720RootFileSetup {
public:
    V1720RootFileSetup(const std::string& outputFileName, const std::string& treeName,
            const Int_t& numSamples);
    ~V1720RootFileSetup();

    void setupBranches(const std::string& branch1, const std::string& branch2);

    // Members to hold branch data
    TFile* m_file;
    TTree* m_tree;

    std::vector<Float_t> m_ch1;
    std::vector<Float_t> m_ch2;
    Int_t m_numSamples;


private:
    std::string m_outputFileName;
    std::string m_treeName;
};

#endif // V1720ROOTFILESETUP_H