#include <Rtypes.h>
#include <TROOT.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TVirtualPad.h>

R__LOAD_LIBRARY({CMAKE_INSTALL_PREFIX}/lib/libV1720.so)

#include "V1720EvioDecode.h"

using namespace std;

void testEvio() {
    const string evioFile = "example.evio.0";
    V1720EvioDecode decoder(evioFile);
    decoder.decode();
}