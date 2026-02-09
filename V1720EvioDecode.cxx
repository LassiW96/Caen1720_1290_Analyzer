#include "V1720EvioDecode.h"

#include <Rtypes.h>
#include <iostream>

using namespace std;

V1720EvioDecode::V1720EvioDecode(const string& eviofile) : 
    m_evioFile(eviofile), m_ch1(), m_ch2()
{
    tbank.start = nullptr;
    tbank.evTS = nullptr;
    tbank.evType = nullptr;
}

void V1720EvioDecode::decode() 
{
    int handle, status;

    status = evOpen((char*)m_evioFile.c_str(), (char*)"r", &handle);
    if (status != S_SUCCESS) {
        cout << "Unable to open file " << m_evioFile << endl;
        return;
    }

    uint32_t *buf = nullptr;
    uint32_t bufLen = 0;
    int buffer_count = 0;

    while (evReadAlloc(handle, &buf, &bufLen) !=EOF)
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

        constexpr int dataStart = 6; // 4 words header + 2 words run info (if present) need to figure this out
        int numWords = bankWords;          // total payload words
        int nData    = numWords - dataStart;  // waveform words only
        int firsthalf = dataStart + nData / 2;

        m_ch1.resize(nData);
        m_ch2.resize(nData);

        // ---- First half → ch1 ----
        for (int i = dataStart; i < firsthalf; i++) {
            uint32_t w = tbank.start[i];
            uint16_t low  = w & 0xffff;
            uint16_t high = (w >> 16) & 0xffff;

            int idx = (i - dataStart) * 2;
            m_ch1[idx]     = low;
            m_ch1[idx + 1] = high;
        }

        // ---- Second half → ch2 ----
        for (int i = firsthalf; i < dataStart + nData; i++) {
            uint32_t w = tbank.start[i];
            uint16_t low  = w & 0xffff;
            uint16_t high = (w >> 16) & 0xffff;

            int idx = (i - firsthalf) * 2;
            m_ch2[idx]     = low;
            m_ch2[idx + 1] = high;
        }

        free(buf);
        buf = nullptr;
    }
    evClose(handle);
    cout << "Total buffers processed: " << buffer_count << endl;
}

vector<uint16_t> V1720EvioDecode::getChannelData(int channel) const
{
    if (channel == 1) {
        return m_ch1;
    } else if (channel == 2) {
        return m_ch2;
    } else {
        cout << "Invalid channel number: " << channel << ". Valid channels are 1 and 2." << endl;
        return {};
    }
}

int V1720EvioDecode::trigBankDecode(uint32_t *tb, int blkSize)
{
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
uint32_t* V1720EvioDecode::findBank(uint32_t* ptr, uint32_t* end, uint16_t wantedTag)
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

ClassImp(V1720EvioDecode)