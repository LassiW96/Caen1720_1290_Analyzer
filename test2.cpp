#include "Rtypes.h"
#include <cstdint>
R__ADD_INCLUDE_PATH(/home/lasitha/EVIO/evio/Linux-x86_64/include)
R__LOAD_LIBRARY(/home/lasitha/EVIO/evio/Linux-x86_64/lib/libevio.so)

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <evio.h>
#include <TTree.h>
#include <TFile.h>

#define TRIG_TAG 1720 // Trigger tag (ADC Module I guess)

using namespace std;

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

TBOBJ tbank;

int trigBankDecode(uint32_t *tb, int blkSize) {
    memset((void *)&tbank, 0, sizeof(TBOBJ));

    tbank.start = tb;
    tbank.blksize = blkSize;
    tbank.len = tb[0] + 1;
    tbank.tag = (tb[1] & 0xffff0000) >> 16;
    tbank.runInfo = (tb[1] & 0x3f00) >> 8;
    tbank.nrocs = (tb[1] & 0xff);
    tbank.evtNum = tb[3];

    if ((tbank.tag) & 1)
        tbank.withTimeStamp = 1;
    if ((tbank.tag) & 2)
        tbank.withRunInfo = 1;

    if(tbank.withTimeStamp) {
        tbank.evTS = (uint64_t *)&tb[5];
        if(tbank.withRunInfo) {
            tbank.evType = (uint16_t *)&tb[5 + 2*blkSize + 3];
        }else{
            tbank.evType = (uint16_t *)&tb[5 + 2*blkSize + 1];
        }
    }else{
        tbank.evTS = NULL;
        if(tbank.withRunInfo) {
            tbank.evType = (uint16_t *)&tb[5 + 3];
        }else{
            tbank.evType = (uint16_t *)&tb[5 + 1];
        }
    }

    return tbank.len;
}

// Helper to find trigger bank
uint32_t* findBank(uint32_t* ptr, uint32_t* end, uint16_t wantedTag)
{
    while (ptr < end) {
        uint32_t len  = ptr[0];
        uint32_t head = ptr[1];

        uint16_t tag  = (head >> 16) & 0xffff;
        uint8_t  type = (head >> 8)  & 0x3f;

        uint32_t* payload = ptr + 2;
        uint32_t* next    = ptr + len + 1;

        // Found it
        if (tag == wantedTag) {
            return payload;
        }

        // Container → recurse
        if (type == 0x10 || type == 0xe) {
            uint32_t* found = findBank(payload, next, wantedTag);
            if (found) return found;
        }

        ptr = next;
    }
    return nullptr;
}

void test2()
{
    const char *inputFileName = "ADCTest_1099.evio.0";
    int handle, status;

    status = evOpen((char*)inputFileName, (char*)"r", &handle);
    if (status != S_SUCCESS) {
        cout << "Unable to open file " << inputFileName << endl;
        return;
    }

    uint32_t *buf = nullptr;
    uint32_t bufLen = 0;

    int buffer_count = 0;
    const int MAX_BUFFERS = 5;

    while (buffer_count < MAX_BUFFERS &&
           evReadAlloc(handle, &buf, &bufLen) == S_SUCCESS)
    {
        buffer_count++;

        uint32_t* eventStart = buf + 2;               // skip outer EVIO header
        uint32_t* eventEnd   = buf + bufLen + 1;

        // 🔎 Find CAEN 1720 bank
        uint32_t* adcPayload =
            findBank(eventStart, eventEnd, 1720);

        if (!adcPayload) {
            cout << "Event " << buffer_count
                 << ": CAEN 1720 bank not found" << endl;
            free(buf);
            continue;
        }

        // Decode trigger/bank header you already wrote
        int bankWords = trigBankDecode(adcPayload - 2, bufLen);

        cout << "Event number: " << tbank.evtNum
             << ", Tag: " << tbank.tag
             << ", Number of ROCs: " << tbank.nrocs
             << ", Bank Length (words): " << bankWords
             << endl;

        constexpr int dataStart = 6; // 4 words header + 2 words run info (if present) need to figure this out
        int numWords = bankWords;          // total payload words
        int nData    = numWords - dataStart;  // waveform words only
        int firsthalf = dataStart + nData / 2;

        // uint16_t ch1[nData];
        // uint16_t ch2[nData];

        vector<uint16_t> ch1(nData);
        vector<uint16_t> ch2(nData);

        // ---- First half → ch1 ----
        for (int i = dataStart; i < firsthalf; i++) {
            uint32_t w = tbank.start[i];
            uint16_t low  = w & 0xffff;
            uint16_t high = (w >> 16) & 0xffff;

            int idx = (i - dataStart) * 2;
            ch1[idx]     = low;
            ch1[idx + 1] = high;

            if (idx < 3070 && idx > 3030) { // Print first few samples for verification
                cout << "ch1 Sample " << (idx / 2) << ": " << low << ", " << high << endl;
            }
        }
        cout << "Size of first half (ch1): " << (firsthalf - dataStart) * 2 << " samples" << endl;
        cout << "Using nData: " << nData << endl;

        free(buf);
        buf = nullptr;
    }

    evClose(handle);

    cout << "Total buffers processed: " << buffer_count << endl;
}
