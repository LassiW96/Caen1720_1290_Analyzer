#ifndef TIDECODE_H
#define TIDECODE_H

#include "Rtypes.h"
#include <RtypesCore.h>
#include <cstdint>
#include <evio.h>
#include <memory>
#include <string>

#include "TIRootFileSetup.h"

// Shared trigger-bank descriptor (same layout as used in V1720/V1290)
typedef struct tiTrigBankObject {
  int blksize;    /* total number of triggers in the block */
  uint16_t tag;   /* Trigger Bank Tag ID */
  uint16_t nrocs; /* Number of ROC banks */
  uint32_t len;   /* Total length (words) including header */
  int withTimeStamp;
  int withRunInfo;
  uint64_t evtNum;
  uint64_t runInfo;
  uint32_t *start;
  uint64_t *evTS;
  uint16_t *evType;
} TI_TBOBJ;

class TIDecode {
public:
  TIDecode(const std::string &evioFile, const std::string &outputDir = "");
  ~TIDecode();

  void decode();

private:
  // EVIO helpers
  uint32_t *findBank(uint32_t *ptr, uint32_t *end, uint16_t wantedTag);
  int trigBankDecode(uint32_t *tb, int blkSize);

  // Per-event decoders
  void decodeADC(uint32_t *payload, int nWords);
  void decodeTDC(uint32_t *payload, int nWords);

  // Members
  std::string m_evioFile;
  std::unique_ptr<TIRootFileSetup> m_rootFileSetup;
  TI_TBOBJ m_tbank;

  // V1720 ADC configuration
  Int_t m_numChannels = 2;   // number of ADC channels
  Int_t m_numSamples = 4096; // samples per channel per event

  // V1290 TDC constants
  static constexpr double kTdcResolutionPs = 100.0; // ps per count

  ClassDef(TIDecode, 1)
};

#endif // TIDECODE_H
