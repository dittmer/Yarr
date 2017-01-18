#ifndef EMURXCORE_H
#define EMURXCORE_H

// #################################
// # Author: Timon Heim
// # Email: timon.heim at cern.ch
// # Project: Yarr
// # Description: Emulator Transmitter 
// # Comment: 
// # Date: Jan 2017
// ################################

#include <iostream>

#include "RxCore.h"

class EmuRxCore : public RxCore {
    public:
        EmuRxCore();
        ~EmuRxCore();
        
        void setRxEnable(uint32_t val) {}
        void maskRxEnable(uint32_t val, uint32_t mask) {}

        RawData* readData() {return NULL;}
        
        uint32_t getDataRate() {return 0;}
        uint32_t getCurCount() {return 0;}
        bool isBridgeEmpty() {return true;}

    private:

};

#endif