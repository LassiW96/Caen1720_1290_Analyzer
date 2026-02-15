#include <Rtypes.h>
#include <TROOT.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TVirtualPad.h>

R__LOAD_LIBRARY({CMAKE_INSTALL_PREFIX}/lib/libV1720.so)

#include "V1720EvioDecode.h"

using namespace std;

void testEvio() {
    const string evioFile = "path/to/example.evio.0";

    // Optional: specify an output directory for the ROOT file, if not provided, it will be
    // created in the same directory as the project source
    const string outputDir = "path/to/output"; 
    V1720EvioDecode decoder(evioFile, outputDir);

    decoder.decode();
}