#include <Rtypes.h>
#include <TROOT.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TVirtualPad.h>

R__ADD_INCLUDE_PATH(/home/lasitha/EVIO/evio/Linux-x86_64/include)
R__LOAD_LIBRARY(/home/lasitha/EVIO/evio/Linux-x86_64/lib/libevio.so)
R__LOAD_LIBRARY(/home/lasitha/MyFiles/Root/proj2-SDK/install/lib/libV1720.so)

#include "V1720FileSetup.h"

using namespace std;

void test() {

    const string fname = "Tree_1101.root";
    const string tree = "t1";
    const string numSampleBranch = "numSamples";
    const string branch1 = "ch1";
    const string branch2 = "ch2";

    V1720FileSetup Set1(fname, tree);

    Set1.setupBranch(branch1, numSampleBranch);
    Set1.setupBranch(branch2, numSampleBranch);

    vector<vector<Float_t>> allSamples1 = Set1.getAllSamples(branch1);
    vector<vector<Float_t>> allSamples2 = Set1.getAllSamples(branch2);

    Set1.processMinSamples(branch1, 3072, 100);
    // Set1.printMinSamples(branch1, 30);
    // Set1.printMinSampleidxs(branch1, 30);
    // Set1.printAllSamples(branch1, 2, 3020, 3050);

    vector<Float_t> minSamples1 = Set1.getMinSamples(branch1);

    TH1F* hist1 = new TH1F("Hist1", "Min Amplitude;Amplitude;Count", 100, 0, 3000);

    for (auto& val : minSamples1) {
        hist1->Fill(val);
    }

    TCanvas* c1 = new TCanvas("c1", "Canvas1", 600, 500);

    hist1->Draw();
    //c1->SaveAs("Hist1.png");
}