#include <Rtypes.h>
#include <TROOT.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TVirtualPad.h>
// #include <iostream>
// #include <vector>

R__LOAD_LIBRARY(/home/lasitha/MyFiles/Root/proj2-SDK/install/lib/libV1720.so)

//#include "V1720FileSetup.h"
#include "V1720EvioDecode.h"

using namespace std;

void testEvio() {
    const string evioFile = "ADCTest_1102.evio.0";
    V1720EvioDecode decoder(evioFile);
    decoder.decode();
}