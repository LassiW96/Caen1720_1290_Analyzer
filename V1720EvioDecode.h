#ifndef V1720EVIODECODE_H
#define V1720EVIODECODE_H

#include "Rtypes.h"
#include <RtypesCore.h>
#include <cstdint>
#include <memory>
#include <string>
#include <evio.h>
// #include <vector>

#include "V1720RootFileSetup.h"

typedef struct trigBankObject {
    int      blksize;              /* total number of triggers in the Bank */
    uint16_t tag;                  /* Trigger Bank Tag ID = 0xff2x */
    uint16_t nrocs;                /* Number of ROC Banks in the Event Block (val = 1-256) */
    uint32_t len;                  /* Total Length of the Trigger Bank - including Bank header */
    int      withTimeStamp;        /* =1 if Time Stamps are available */
    int      withRunInfo;          /* =1 if Run Informaion is available - Run # and Run Type */
    uint64_t evtNum;               /* Starting Event # of the Block */
    uint64_t runInfo;              /* Run Info Data */
    uint32_t *start;               /* Pointer to start of the Trigger Bank */
    uint64_t *evTS;                /* Pointer to the array of Time Stamps */
    uint16_t *evType;              /* Pointer to the array of Event Types */
} TBOBJ;

class V1720EvioDecode {
public:
    V1720EvioDecode(const std::string& eviofile, const std::string& outputdir = "");
    ~V1720EvioDecode();

    void decode();

private:
    TBOBJ tbank;
    int trigBankDecode(uint32_t *tb, int blkSize);
    // Helper to find trigger bank
    uint32_t* findBank(uint32_t* ptr, uint32_t* end, uint16_t wantedTag);

    // Members
    std::string m_evioFile;

    std::unique_ptr<V1720RootFileSetup> m_rootFileSetup;

    // Hard coded for now, can be made configurable later
    Int_t m_numChannels = 2; // default, can be set via constructor or other method
    Int_t m_numSamples = 4096; // default, can be set via constructor or other method

    ClassDef(V1720EvioDecode, 1)
};


#endif // V1720EVIOTOROOT_H