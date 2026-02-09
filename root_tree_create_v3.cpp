/************************************************* * Simple test 
program for opening and reading * * an existing CODA event format file.  
* * * * CODA EVIO Version 4 or later * * * * Author: David Abbott CEBAF 
* * * * Arguments to the routine are: * * * * evt <filename> [nevents] * 
* * * * Modified to create a root tree using an evio file by Lasitha G. Welianga 
* * * * on 01/05/2025
* * **************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <evio.h>
#include <TTree.h>
#include <TFile.h>

#define MAX_BLOCK_SIZE 255
#define MAX_ROCS       255

#define MIN(a,b) (a<b)? a : b

int
trigBankDecode(uint32_t *tb, int blkSize);

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

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Incorrect number of arguments:\n");
        printf("  usage: %s <input filename> <output ROOT filename> [-t -r]\n", argv[0]);
        printf("  optional: -t Print Trigger Bank\n");
        printf("            -r Print ROC Banks\n");
        exit(-1);
    }

    char *inputFileName = argv[1];
    char *outputFileName = argv[2];

    int ptb = 0, pr = 0;
    if (argc > 3) {
        if (strncmp(argv[3], "-t", 2) == 0) ptb = 1;
        if (strncmp(argv[3], "-r", 2) == 0) pr = 1;
    }
    if (argc >= 5) {
        if (strncmp(argv[4], "-t", 2) == 0) ptb = 1;
        if (strncmp(argv[4], "-r", 2) == 0) pr = 1;
    }

    int handle, status;
    if ((status = evOpen(inputFileName, "r", &handle)) < 0) {
        printf("Unable to open file %s status = %d\n", inputFileName, status);
        exit(-1);
    } else {
        printf("Opened %s for reading, output will be written to %s\n", inputFileName, outputFileName);
    }

    // Create a ROOT file with the provided output file name
    TFile f(outputFileName, "recreate");
    TTree t1("t1", "a simple Tree with simple variables");

    Float_t ch1[4096] = {0.0};
    Float_t ch2[4096] = {0.0};
    const Int_t knumSamples = 4096;
    Int_t numSamples = knumSamples;

    t1.Branch("numSamples", &numSamples, "numSamples/I");
    t1.Branch("ch1", ch1, "ch1[numSamples]/F");
    t1.Branch("ch2", ch2, "ch2[numSamples]/F");

    uint32_t *buf, bufLen;
    int buffer_count = 0;

    while ((status = evReadAlloc(handle, &buf, &bufLen)) != EOF) {
        buffer_count++;

        if (buffer_count <= 2) {
            free(buf);
            continue;
        }

        int num = buf[0] + 1;
        int firsthalf = 18 + (num - 18) / 2;

        for (int i = 18; i < firsthalf; i++) {
            int high_byte1 = (buf[i] >> 16) & 0xffff;
            int low_byte1 = buf[i] & 0xffff;
            ch1[(i - 18) * 2] = low_byte1;
            ch1[(i - 18) * 2 + 1] = high_byte1;
        }

        for (int i = firsthalf; i < num; i++) {
            int high_byte2 = (buf[i] >> 16) & 0xffff;
            int low_byte2 = buf[i] & 0xffff;
            ch2[(i - firsthalf) * 2] = low_byte2;
            ch2[(i - firsthalf) * 2 + 1] = high_byte2;
        }

        t1.Fill();
        free(buf);
    }

    if (status == EOF) {
        printf("Found end-of-file\n");
    } else if (status != 0) {
        printf("Error reading file (status = %d, quitting)\n", status);
    }

    t1.Write();
    printf("Processed %d buffers\n", buffer_count);
    f.Close();
    evClose(handle);
    return 0;
}

int trigBankDecode(uint32_t *tb, int blkSize) {
    memset((void *)&tbank, 0, sizeof(TBOBJ));

    tbank.start = tb;
    tbank.blksize = blkSize;
    tbank.len = tb[0] + 1;
    tbank.tag = (tb[1] & 0xffff0000) >> 16;
    tbank.nrocs = (tb[1] & 0xff);
    tbank.evtNum = tb[3];

    if ((tbank.tag) & 1)
        tbank.withTimeStamp = 1;
    if ((tbank.tag) & 2)
        tbank.withRunInfo = 1;

    return tbank.len;
}
