#ifndef FEI4PIXELTHRESHOLDTUNE_H
#define FEI4PIXELTHRESHOLDTUNE_H

// #################################
// # Author: Timon Heim
// # Email: timon.heim at cern.ch
// # Project: Yarr
// # Description: Fei4 Threshold Scan
// # Comment: Nothing fancy
// ################################

#include "ScanBase.h"

#include "AllStdActions.h"
#include "AllFei4Actions.h"

class Fei4PixelThresholdTune : public ScanBase {
    public:
        Fei4PixelThresholdTune(Fei4 *fe, TxCore *tx, RxCore *rx, ClipBoard<RawData> *data);
        
        void init();
        void preScan();
        void postScan() {}

    private:
        enum MASK_STAGE mask;
        enum DC_MODE dcMode;
        unsigned numOfTriggers;
        double triggerFrequency;
        unsigned triggerDelay;
        unsigned minVcal;
        unsigned maxVcal;
        unsigned stepVcal;
        
        double target;
        bool verbose;
};

#endif
