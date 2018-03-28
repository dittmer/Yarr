// #################################
// # Author: Benjamin Nachman and Timon Heim 
// # Email: bnachman / timon.heim at cern.ch
// # Project: Yarr
// # Description: Fe65p2 TOT Scan
// # Data: 2016-August-17
// # Comment: TOT for a known charge
// ################################

#include "Fe65p2XtalkScan.h"

Fe65p2XtalkScan::Fe65p2XtalkScan(Bookkeeper *k) : ScanBase(k) {

}

void Fe65p2XtalkScan::init() {

    // Loop 1: Mask Staging
    std::shared_ptr<Fe65p2XtalkMaskLoop> maskStaging(new Fe65p2XtalkMaskLoop);

    // Loop 2: Double Columns
    std::shared_ptr<Fe65p2QcLoop> dcLoop(new Fe65p2QcLoop);
    
    // Loop 3: Trigger
    std::shared_ptr<Fe65p2TriggerLoop> triggerLoop(new Fe65p2TriggerLoop);

    // Loop 4: Data gatherer
    std::shared_ptr<StdDataLoop> dataLoop(new StdDataLoop);
    dataLoop->setVerbose(false);
    dataLoop->connect(g_data);

    this->addLoop(maskStaging);
    this->addLoop(dcLoop);
    this->addLoop(triggerLoop);
    this->addLoop(dataLoop);
    
    engine.init();
}

void Fe65p2XtalkScan::preScan() {

  g_fe65p2->setValue(&Fe65p2::PlsrDac, g_fe65p2->toVcal(2000));
  //g_fe65p2->setValue(&Fe65p2::PlsrDac, 300); // Set high injection
    g_fe65p2->configDac();
    
    for(unsigned i=0; i<16; i++) {
        //g_fe65p2->PixConf(i).setAll(0);
    }
    g_fe65p2->configurePixels();
    
    g_fe65p2->setLatency(60+5);
    g_fe65p2->setValue(&Fe65p2::TestHit, 0x0);
    //g_fe65p2->setValue(&Fe65p2::Vthin1Dac, 100); // Set Threshold not too low, not too high
    //g_fe65p2->setValue(&Fe65p2::Vthin2Dac, 50); // Set Threshold not too low, not too high
    g_fe65p2->enAnaInj();
    g_fe65p2->setValue(&Fe65p2::Latency, 60);
    g_fe65p2->configureGlobal();

    while(g_tx->isCmdEmpty() == 0);
}
