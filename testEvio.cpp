#include <Rtypes.h>
#include <TROOT.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TVirtualPad.h>
#include <cstdint>
#include <iostream>
#include <vector>

// R__ADD_INCLUDE_PATH(/home/lasitha/EVIO/evio/Linux-x86_64/include)
// R__LOAD_LIBRARY(/home/lasitha/EVIO/evio/Linux-x86_64/lib/libevio.so)
R__LOAD_LIBRARY(/home/lasitha/MyFiles/Root/proj2-SDK/install/lib/libV1720.so)

//#include "V1720FileSetup.h"
#include "V1720EvioDecode.h"

using namespace std;

void testEvio() {
    const string evioFile = "ADCTest_1099.evio.0";
    V1720EvioDecode decoder(evioFile);
    decoder.decode();

    auto ch1 = decoder.getChannelData(1);

    for (int i = 0; i < 10; ++i) {
        cout << "Channel 1, Sample " << i << ": " << ch1[i] << endl;
    }
}