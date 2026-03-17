#include "V1290EvioDecode.h"

#include <Rtypes.h>
#include <RtypesCore.h>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>

using namespace std;

// CAEN V1290A data type identifiers (bits [31:27])
static constexpr uint32_t DATA_TYPE_MASK = 0xF8000000;
static constexpr int TYPE_TDC_MEASUREMENT = 0; // 0b00000
// static constexpr int TYPE_TDC_HEADER        = 1;   // 0b00001
// static constexpr int TYPE_TDC_TRAILER       = 3;   // 0b00011
// static constexpr int TYPE_TDC_ERROR         = 4;   // 0b00100
static constexpr int TYPE_GLOBAL_HEADER = 8;   // 0b01000
static constexpr int TYPE_GLOBAL_TRAILER = 16; // 0b10000
// static constexpr int TYPE_TRIGGER_TIME      = 17;  // 0b10001
static constexpr int TYPE_FILLER = 24; // 0b11000

V1290EvioDecode::V1290EvioDecode(const string &eviofile,
                                 const string &outputdir)
    : m_evioFile(eviofile), m_rootFileSetup(nullptr) {
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
  string outputFileName = p.string() + "_tdc_decoded.root";

  std::filesystem::path fullOutputPath;

  if (outputdir.empty()) {
    fullOutputPath = outputFileName;
  } else {
    fullOutputPath = std::filesystem::path(outputdir) / outputFileName;
    std::filesystem::create_directories(fullOutputPath.parent_path());
  }

  m_rootFileSetup =
      make_unique<V1290RootFileSetup>(fullOutputPath.string(), "tdc");
}

void V1290EvioDecode::decode() {
  int handle, status;

  status = evOpen((char *)m_evioFile.c_str(), (char *)"r", &handle);
  if (status != S_SUCCESS) {
    cout << "Unable to open file " << m_evioFile << endl;
    return;
  }

  uint32_t *buf = nullptr;
  uint32_t bufLen = 0;
  int buffer_count = 0;
  int skipped_count = 0;
  int event_count = 0;

  m_rootFileSetup->setupBranches();

  while (evReadAlloc(handle, &buf, &bufLen) != EOF) {
    buffer_count++;

    uint32_t *eventStart = buf + 2; // skip outer EVIO header
    uint32_t *eventEnd = buf + bufLen + 1;

    // Find CAEN V1290A TDC bank (tag = 1190)
    uint32_t *tdcPayload = findBank(eventStart, eventEnd, 1190);

    if (!tdcPayload) {
      skipped_count++;
      free(buf);
      continue;
    }

    // Decode EVIO bank header
    int bankWords = trigBankDecode(tdcPayload - 2, bufLen);

    // Walk the TDC data words and collect measurements per channel
    std::map<int, int32_t> hits; // channel -> measurement

    for (int i = 2; i < bankWords; i++) {
      uint32_t w = tbank.start[i];
      int datatype = (w & DATA_TYPE_MASK) >> 27;

      if (datatype == TYPE_TDC_MEASUREMENT) {
        // bit 26 = edge (0=leading, 1=trailing)
        // bits [25:21] = channel (5-bit)
        // bits [20:0]  = measurement (21-bit)
        int edge = (w & 0x04000000) >> 26;
        int channel = (w & 0x03E00000) >> 21;
        int32_t measurement = (w & 0x001FFFFF);

        // Only use leading-edge measurements
        if (edge == 0) {
          hits[channel] = measurement;
        }
      } else if (datatype == TYPE_GLOBAL_HEADER) {
        // Start of a TDC event — clear hits for this event
        hits.clear();
      } else if (datatype == TYPE_GLOBAL_TRAILER) {
        // End of a TDC event — compute delta-t and fill tree
        if (hits.count(0) && (hits.count(1) || hits.count(2))) {
          event_count++;
          m_rootFileSetup->m_event.clear();
          m_rootFileSetup->m_event.eventNumber = event_count;

          if (hits.count(1)) {
            int32_t dt_counts = hits[0] - hits[1];
            m_rootFileSetup->m_event.delta_t_ch0_ch1 =
                static_cast<Float_t>(dt_counts * kTdcResolutionPs / 1000.0);
          }

          if (hits.count(2)) {
            int32_t dt_counts = hits[0] - hits[2];
            m_rootFileSetup->m_event.delta_t_ch0_ch2 =
                static_cast<Float_t>(dt_counts * kTdcResolutionPs / 1000.0);
          }

          m_rootFileSetup->m_tree->Fill();
        }
        hits.clear();
      } else if (datatype == TYPE_FILLER) {
        // Skip filler words
        continue;
      }
      // All other types (TDC header/trailer, trigger time, errors) are ignored
    }

    free(buf);
    buf = nullptr;
  }

  m_rootFileSetup->m_tree->Write("", TObject::kOverwrite);
  m_rootFileSetup->m_file->Close();
  evClose(handle);

  cout << "\nFinished processing TDC file: " << m_evioFile << endl;
  cout << "Total buffers processed: " << buffer_count << endl;
  cout << "Skipped non-TDC buffers: " << skipped_count << endl;
  cout << "TDC events decoded:      " << event_count << endl;
  cout << "-----------------------------------------------------\n" << endl;
}

int V1290EvioDecode::trigBankDecode(uint32_t *tb, int blkSize) {
  memset((void *)&tbank, 0, sizeof(TBOBJ1290));

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

  if (tbank.withTimeStamp) {
    tbank.evTS = (uint64_t *)&tb[5];
    if (tbank.withRunInfo) {
      tbank.evType = (uint16_t *)&tb[5 + 2 * blkSize + 3];
    } else {
      tbank.evType = (uint16_t *)&tb[5 + 2 * blkSize + 1];
    }
  } else {
    tbank.evTS = NULL;
    if (tbank.withRunInfo) {
      tbank.evType = (uint16_t *)&tb[5 + 3];
    } else {
      tbank.evType = (uint16_t *)&tb[5 + 1];
    }
  }
  return tbank.len;
}

uint32_t *V1290EvioDecode::findBank(uint32_t *ptr, uint32_t *end,
                                    uint16_t wantedTag) {
  while (ptr < end) {
    uint32_t len = ptr[0];
    uint32_t head = ptr[1];

    uint16_t tag = (head >> 16) & 0xffff;
    uint8_t type = (head >> 8) & 0x3f;

    uint32_t *payload = ptr + 2;
    uint32_t *next = ptr + len + 1;

    // Found it
    if (tag == wantedTag) {
      return payload;
    }

    // Container → recurse
    if (type == 0x10 || type == 0xe) {
      uint32_t *found = findBank(payload, next, wantedTag);
      if (found)
        return found;
    }

    ptr = next;
  }
  return nullptr;
}

V1290EvioDecode::~V1290EvioDecode() = default;

ClassImp(V1290EvioDecode)
