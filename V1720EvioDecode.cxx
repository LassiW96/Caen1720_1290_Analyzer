#include "V1720EvioDecode.h"

#include <Rtypes.h>
#include <RtypesCore.h>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

V1720EvioDecode::V1720EvioDecode(const string& eviofile, const string& outputdir) : 
    m_evioFile(eviofile), m_rootFileSetup(nullptr)
{
    tbank.start = nullptr;
    tbank.evTS = nullptr;
    tbank.evType = nullptr;
    
    if (!std::filesystem::exists(m_evioFile)) {
        throw std::runtime_error("Input EVIO file does not exist: " + m_evioFile);
    }

    filesystem::path p(m_evioFile);
    while (p.has_extension()) {
    p = p.stem();
    }
    string outputFileName = p.string() + "_decoded.root";

    std::filesystem::path fullOutputPath;

    if (outputdir.empty()) {
        fullOutputPath = outputFileName;  // same directory as executable
    } else {
        fullOutputPath = std::filesystem::path(outputdir) / outputFileName;
        std::filesystem::create_directories(fullOutputPath.parent_path());
    }

    m_rootFileSetup = make_unique<V1720RootFileSetup>(fullOutputPath.string(), 
            "t1", 
            m_numChannels, 
            m_numSamples);
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
    int skipped_count = 0;

    m_rootFileSetup->setupBranches();

    while (evReadAlloc(handle, &buf, &bufLen) !=EOF)
    {
        buffer_count++;

        uint32_t* eventStart = buf + 2;               // skip outer EVIO header
        uint32_t* eventEnd   = buf + bufLen + 1;

        // 🔎 Find CAEN 1720 bank
        uint32_t* adcPayload =
            findBank(eventStart, eventEnd, 1720);

        if (!adcPayload) {
            // cout << "Event " << buffer_count << ": Not a Payload Bank" << endl;
            skipped_count++;
            free(buf);
            continue;
        }

        // Decode trigger/bank header you already wrote
        int bankWords = trigBankDecode(adcPayload - 2, bufLen);

        constexpr int dataStart = 6; // 4 words header + 2 words run info (if present) need to figure this out
        int numWords = bankWords;          // total payload words
        int nData    = numWords - dataStart;  // waveform words only
        int firsthalf = dataStart + nData / 2;

        // ---- First half → ch1 ----
        auto& ch0 = m_rootFileSetup->m_event.channels[0];
        Float_t integral0 = 0;
        Float_t peak0 = 1e9;
        Float_t peakIdx0 = 0;

        for (int i = dataStart; i < firsthalf; i++) {
            uint32_t w = tbank.start[i];
            uint16_t low  = w & 0xffff;
            uint16_t high = (w >> 16) & 0xffff;

            int idx = (i - dataStart) * 2;
            Float_t sample0 = static_cast<Float_t>(low);
            Float_t sample1 = static_cast<Float_t>(high);
            ch0.samples[idx] = sample0;
            ch0.samples[idx + 1] = sample1; 
            integral0 += sample0 + sample1;
            if (sample0 < peak0) {
                peak0 = sample0;
                peakIdx0 = idx;
            }
            if (sample1 < peak0) {
                peak0 = sample1;
                peakIdx0 = idx + 1;
            }
        }
        ch0.integral = integral0;
        ch0.peak = peak0;
        ch0.peakIdx = peakIdx0;

        // ---- Second half → ch2 ----
            
        auto& ch1 = m_rootFileSetup->m_event.channels[1];
        Float_t integral1 = 0;
        Float_t peak1 = 1e9;
        Float_t peakIdx1 = 0;
        for (int i = firsthalf; i < dataStart + nData; i++) {
            uint32_t w = tbank.start[i];
            uint16_t low  = w & 0xffff;
            uint16_t high = (w >> 16) & 0xffff;

            int idx = (i - firsthalf) * 2;
            ch1.samples[idx] = static_cast<Float_t>(low);
            ch1.samples[idx + 1] = static_cast<Float_t>(high);
            Float_t sample0 = static_cast<Float_t>(low);
            Float_t sample1 = static_cast<Float_t>(high);
            integral1 += low + high;
            if (sample0 < peak1) {
                peak1 = sample0;
                peakIdx1 = idx;
            }
            if (sample1 < peak1) {
                peak1 = sample1;
                peakIdx1 = idx + 1;
            }
        }
        ch1.integral = integral1;
        ch1.peak = peak1;
        ch1.peakIdx = peakIdx1;

        m_rootFileSetup->m_tree->Fill();

        free(buf);
        buf = nullptr;
    }
    m_rootFileSetup->m_tree->Write("", TObject::kOverwrite);
    m_rootFileSetup->m_file->Close();
    evClose(handle);

    cout << "\nFinished processing file: " << m_evioFile << endl;
    cout << "Total buffers processed: " << buffer_count << endl;
    cout << "Skipped non-payload buffers: " << skipped_count << endl;
    cout << "-----------------------------------------------------\n" << endl;
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

V1720EvioDecode::~V1720EvioDecode() = default;

ClassImp(V1720EvioDecode)