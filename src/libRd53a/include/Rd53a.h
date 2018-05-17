#ifndef RD53A_H
#define RD53A_H


// #################################
// # Author: Timon Heim
// # Email: timon.heim at cern.ch
// # Project: Yarr
// # Comment: RD53A base class
// # Date: Jun 2017
// #################################

#include <iostream>
#include <chrono>
#include <thread>
#include <tuple>
#include <vector>

#include "FrontEnd.h"
#include "TxCore.h"
#include "RxCore.h"
#include "Rd53aCfg.h"
#include "Rd53aCmd.h"

class Rd53a : public FrontEnd, public Rd53aCfg, public Rd53aCmd {
    public:
        Rd53a();
        Rd53a(TxCore *arg_core);
        Rd53a(TxCore *arg_core, unsigned arg_channel);
        Rd53a(TxCore *arg_core, unsigned arg_txchannel, unsigned arg_rxchannel);

	virtual ~Rd53a() {}
    
        void init(TxCore *arg_core, unsigned arg_txChannel, unsigned arg_rxChannel);
        void makeGloba() {
            m_chipId = 8;
        }

        void configure();
        void configureInit();
        void configureGlobal();
        void configurePixels();
        void configurePixels(std::vector<std::pair<unsigned, unsigned>> &pixels);

        void writeRegister(Rd53aReg Rd53aGlobalCfg::*ref, uint32_t value);
        void readRegister(Rd53aReg Rd53aGlobalCfg::*ref);
        void writeNamedRegister(std::string name, uint16_t value);
        
        void setInjCharge(double charge, bool sCap=true, bool lCap=true) {
            this->writeRegister((Rd53aReg Rd53aGlobalCfg::*)&Rd53aGlobalCfg::InjVcalDiff, this->toVcal(charge));
        }
        
        void enableCalCol(unsigned col);
        void disableCalCol(unsigned col);

    protected:
    private:
};

#endif
