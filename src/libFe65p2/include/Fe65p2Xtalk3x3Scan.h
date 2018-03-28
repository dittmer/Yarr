// #################################
// # Author: Benjamin Nachman and Timon Heim
// # Email: bnachman / timon.heim at cern.ch
// # Project: Yarr
// # Description: Fe65p2 TOT Scan
// # Data: 2016-August-17
// # Comment: TOT for a 'wide masked' pixel  
// ################################

#ifndef FE65P2XTALK3X3SCAN_H
#define FE65P2XTALK3X3SCAN_H

#include "ScanBase.h"

#include "AllStdActions.h"
#include "AllFe65p2Actions.h"

class Fe65p2Xtalk3x3Scan : public ScanBase {
    public:
        Fe65p2Xtalk3x3Scan(Bookkeeper *k);

        void init();
        void preScan();
        void postScan() {}
    private:
};

#endif
