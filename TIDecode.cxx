#include "TIDecode.h"

#include <Rtypes.h>
#include <RtypesCore.h>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

// V1290 TDC data-type bit masks / identifiers (bits [31:27])
static constexpr uint32_t TI_DATA_TYPE_MASK = 0xF8000000;
static constexpr int TI_TYPE_TDC_MEASUREMENT = 0; // 0b00000
static constexpr int TI_TYPE_GLOBAL_HEADER = 8;   // 0b01000
static constexpr int TI_TYPE_GLOBAL_TRAILER = 16; // 0b10000
static constexpr int TI_TYPE_FILLER = 24;         // 0b11000

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
TIDecode::TIDecode(const string &evioFile, const string &outputDir)
    : m_evioFile(evioFile), m_rootFileSetup(nullptr) {
  memset(&m_tbank, 0, sizeof(TI_TBOBJ));

  if (!filesystem::exists(m_evioFile))
    throw runtime_error("Input EVIO file does not exist: " + m_evioFile);

  // Build output filename: strip all extensions, append _ti_decoded.root
  filesystem::path p(m_evioFile);
  while (p.has_extension())
    p = p.stem();
  string outputFileName = p.string() + "_ti_decoded.root";

  filesystem::path fullOutputPath;
  if (outputDir.empty()) {
    fullOutputPath = outputFileName;
  } else {
    fullOutputPath = filesystem::path(outputDir) / outputFileName;
    filesystem::create_directories(fullOutputPath.parent_path());
  }

  m_rootFileSetup = make_unique<TIRootFileSetup>(fullOutputPath.string(), "ti",
                                                 m_numChannels, m_numSamples);
}

TIDecode::~TIDecode() = default;

// ---------------------------------------------------------------------------
// decode() – main event loop
// ---------------------------------------------------------------------------
void TIDecode::decode() {
  int handle, status;
  status = evOpen((char *)m_evioFile.c_str(), (char *)"r", &handle);
  if (status != S_SUCCESS) {
    cout << "Unable to open EVIO file: " << m_evioFile << endl;
    return;
  }

  uint32_t *buf = nullptr;
  uint32_t bufLen = 0;
  int buffer_count = 0;
  int event_count = 0;
  int skipped_count = 0;

  m_rootFileSetup->setupBranches();

  while (evReadAlloc(handle, &buf, &bufLen) != EOF) {
    buffer_count++;

    uint32_t *eventStart = buf + 2; // skip outer EVIO bank header
    uint32_t *eventEnd = buf + bufLen + 1;

    // Find the two payload banks
    uint32_t *adcPayload = findBank(eventStart, eventEnd, 1720);
    uint32_t *tdcPayload = findBank(eventStart, eventEnd, 1190);

    if (!adcPayload && !tdcPayload) {
      skipped_count++;
      free(buf);
      continue;
    }

    event_count++;
    m_rootFileSetup->m_event.clear();
    m_rootFileSetup->m_event.eventNumber = event_count;

    // ---- ADC ----
    if (adcPayload) {
      // trigBankDecode gives us the bank length;
      // adcPayload-2 points at the bank header word
      int bankWords = trigBankDecode(adcPayload - 2, 1);
      decodeADC(adcPayload, bankWords - 2); // subtract 2-word header
    }

    // ---- TDC ----
    if (tdcPayload) {
      int bankWords = trigBankDecode(tdcPayload - 2, 1);
      decodeTDC(tdcPayload, bankWords - 2);
    }

    m_rootFileSetup->m_tree->Fill();

    free(buf);
    buf = nullptr;
  }

  m_rootFileSetup->m_tree->Write("", TObject::kOverwrite);
  m_rootFileSetup->m_file->Close();
  evClose(handle);

  cout << "\nFinished processing file: " << m_evioFile << endl;
  cout << "Total buffers read:       " << buffer_count << endl;
  cout << "Physics events decoded:   " << event_count << endl;
  cout << "Skipped buffers:          " << skipped_count << endl;
  cout << "-------------------------------------------\n" << endl;
}

// ---------------------------------------------------------------------------
// decodeADC – fill per-channel waveform data from V1720 bank payload
// ---------------------------------------------------------------------------
void TIDecode::decodeADC(uint32_t *payload, int nWords) {
  // V1720 payload layout (from existing V1720EvioDecode):
  //   [0]  event header (0xa000xxxx)
  //   [1]  channel mask / group info
  //   [2]  event counter
  //   [3]  trigger time tag
  //   [4..]waveform data: two 12-bit samples packed per 32-bit word
  //        first half = ch0, second half = ch1

  if (nWords < 5)
    return; // too short

  constexpr int dataStart = 4;    // first waveform word
  int nData = nWords - dataStart; // total waveform words
  int halfData = nData / 2;       // words per channel

  auto &ev = m_rootFileSetup->m_event;

  // ---- Channel 0 ----
  auto &ch0 = ev.channels[0];
  Float_t integral0 = 0, peak0 = 1e9f, peakIdx0 = 0;
  for (int i = 0; i < halfData; i++) {
    uint32_t w = payload[dataStart + i];
    uint16_t low = w & 0xffff;
    uint16_t high = (w >> 16) & 0xffff;
    int idx = i * 2;
    ch0.samples[idx] = static_cast<Float_t>(low);
    ch0.samples[idx + 1] = static_cast<Float_t>(high);
    integral0 += ch0.samples[idx] + ch0.samples[idx + 1];
    if (ch0.samples[idx] < peak0) {
      peak0 = ch0.samples[idx];
      peakIdx0 = idx;
    }
    if (ch0.samples[idx + 1] < peak0) {
      peak0 = ch0.samples[idx + 1];
      peakIdx0 = idx + 1;
    }
  }
  ch0.integral = integral0;
  ch0.peak = peak0;
  ch0.peakIdx = peakIdx0;

  // ---- Channel 1 ----
  if (m_numChannels > 1 && (int)ev.channels.size() > 1) {
    auto &ch1 = ev.channels[1];
    Float_t integral1 = 0, peak1 = 1e9f, peakIdx1 = 0;
    for (int i = 0; i < halfData; i++) {
      uint32_t w = payload[dataStart + halfData + i];
      uint16_t low = w & 0xffff;
      uint16_t high = (w >> 16) & 0xffff;
      int idx = i * 2;
      ch1.samples[idx] = static_cast<Float_t>(low);
      ch1.samples[idx + 1] = static_cast<Float_t>(high);
      integral1 += ch1.samples[idx] + ch1.samples[idx + 1];
      if (ch1.samples[idx] < peak1) {
        peak1 = ch1.samples[idx];
        peakIdx1 = idx;
      }
      if (ch1.samples[idx + 1] < peak1) {
        peak1 = ch1.samples[idx + 1];
        peakIdx1 = idx + 1;
      }
    }
    ch1.integral = integral1;
    ch1.peak = peak1;
    ch1.peakIdx = peakIdx1;
  }
}

// ---------------------------------------------------------------------------
// decodeTDC – extract TDC leading-edge hits from V1290 bank payload
//   Reference = first (lowest-numbered) channel that fired in the event.
// ---------------------------------------------------------------------------
void TIDecode::decodeTDC(uint32_t *payload, int nWords) {
  // hits: channel -> raw measurement (leading edge only, first occurrence wins)
  std::map<int, int32_t> hits;

  auto flushHits = [&]() {
    if (hits.empty())
      return;

    auto &ev = m_rootFileSetup->m_event;

    // Reference = lowest-numbered channel that has a hit
    int refCh = hits.begin()->first;
    int32_t refTime = hits.begin()->second;

    ev.refChannel = refCh;
    ev.refRawTime = refTime;

    for (const auto &h : hits) {
      if (h.first == refCh)
        continue;
      int32_t dt_counts = refTime - h.second; // ref - hit
      ev.hit_channels.push_back(h.first);
      ev.delta_t.push_back(
          static_cast<float>(dt_counts * kTdcResolutionPs / 1000.0)); // ns
    }
    hits.clear();
  };

  for (int i = 0; i < nWords; i++) {
    uint32_t w = payload[i];
    int datatype = (w & TI_DATA_TYPE_MASK) >> 27;

    if (datatype == TI_TYPE_GLOBAL_HEADER) {
      hits.clear(); // start fresh for this TDC event
    } else if (datatype == TI_TYPE_TDC_MEASUREMENT) {
      int edge = (w & 0x04000000) >> 26;
      int channel = (w & 0x03E00000) >> 21;
      int32_t meas = (w & 0x001FFFFF);
      // Keep leading-edge only; first hit per channel wins
      if (edge == 0 && hits.find(channel) == hits.end())
        hits[channel] = meas;
    } else if (datatype == TI_TYPE_GLOBAL_TRAILER) {
      flushHits();
    }
    // FILLER and other types are ignored
  }
  // Safety flush in case trailer was missing
  if (!hits.empty())
    flushHits();
}

// ---------------------------------------------------------------------------
// trigBankDecode – populate m_tbank from a bank header pointer
// ---------------------------------------------------------------------------
int TIDecode::trigBankDecode(uint32_t *tb, int blkSize) {
  memset(&m_tbank, 0, sizeof(TI_TBOBJ));
  m_tbank.start = tb;
  m_tbank.blksize = blkSize;
  m_tbank.len = tb[0] + 1; // length word + 1 = total words incl. header
  m_tbank.tag = (tb[1] & 0xffff0000) >> 16;
  m_tbank.runInfo = (tb[1] & 0x3f00) >> 8;
  m_tbank.nrocs = (tb[1] & 0xff);
  m_tbank.evtNum = tb[3];
  if (m_tbank.tag & 1)
    m_tbank.withTimeStamp = 1;
  if (m_tbank.tag & 2)
    m_tbank.withRunInfo = 1;
  if (m_tbank.withTimeStamp) {
    m_tbank.evTS = (uint64_t *)&tb[5];
    m_tbank.evType = m_tbank.withRunInfo ? (uint16_t *)&tb[5 + 2 * blkSize + 3]
                                         : (uint16_t *)&tb[5 + 2 * blkSize + 1];
  } else {
    m_tbank.evTS = nullptr;
    m_tbank.evType =
        m_tbank.withRunInfo ? (uint16_t *)&tb[5 + 3] : (uint16_t *)&tb[5 + 1];
  }
  return m_tbank.len;
}

// ---------------------------------------------------------------------------
// findBank – recursively search EVIO bank tree for a bank with given tag
// ---------------------------------------------------------------------------
uint32_t *TIDecode::findBank(uint32_t *ptr, uint32_t *end, uint16_t wantedTag) {
  while (ptr < end) {
    uint32_t len = ptr[0];
    uint32_t head = ptr[1];

    uint16_t tag = (head >> 16) & 0xffff;
    uint8_t type = (head >> 8) & 0x3f;

    uint32_t *payload = ptr + 2;
    uint32_t *next = ptr + len + 1;

    if (tag == wantedTag)
      return payload;

    // Container banks → recurse
    if (type == 0x10 || type == 0x0e) {
      uint32_t *found = findBank(payload, next, wantedTag);
      if (found)
        return found;
    }

    ptr = next;
  }
  return nullptr;
}

ClassImp(TIDecode)
