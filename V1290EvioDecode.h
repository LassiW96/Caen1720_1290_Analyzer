#ifndef V1290EVIODECODE_H
#define V1290EVIODECODE_H

#include "Rtypes.h"
#include <RtypesCore.h>
#include <cstdint>
#include <evio.h>
#include <memory>
#include <string>

#include "V1290RootFileSetup.h"

typedef struct trigBankObject1290 {
  int blksize;    /* total number of triggers in the Bank */
  uint16_t tag;   /* Trigger Bank Tag ID = 0xff2x */
  uint16_t nrocs; /* Number of ROC Banks in the Event Block (val = 1-256) */
  uint32_t len;   /* Total Length of the Trigger Bank - including Bank header */
  int withTimeStamp; /* =1 if Time Stamps are available */
  int withRunInfo; /* =1 if Run Information is available - Run # and Run Type */
  uint64_t evtNum; /* Starting Event # of the Block */
  uint64_t runInfo; /* Run Info Data */
  uint32_t *start;  /* Pointer to start of the Trigger Bank */
  uint64_t *evTS;   /* Pointer to the array of Time Stamps */
  uint16_t *evType; /* Pointer to the array of Event Types */
} TBOBJ1290;

class V1290EvioDecode {
public:
  V1290EvioDecode(const std::string &eviofile,
                  const std::string &outputdir = "");
  ~V1290EvioDecode();

  void decode();

private:
  TBOBJ1290 tbank;
  int trigBankDecode(uint32_t *tb, int blkSize);
  // Helper to find bank by tag
  uint32_t *findBank(uint32_t *ptr, uint32_t *end, uint16_t wantedTag);

  // Members
  std::string m_evioFile;

  std::unique_ptr<V1290RootFileSetup> m_rootFileSetup;

  // TDC resolution in ps per count
  static constexpr double kTdcResolutionPs = 100.0;

  ClassDef(V1290EvioDecode, 1)
};

#endif // V1290EVIODECODE_H
