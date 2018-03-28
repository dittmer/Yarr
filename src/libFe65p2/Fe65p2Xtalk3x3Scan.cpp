// #################################
// # Author: Benjamin Nachman and Timon Heim 
// # Email: bnachman / timon.heim at cern.ch
// # Project: Yarr
// # Description: Fe65p2 TOT Scan
// # Data: 2016-August-17
// # Comment: TOT for a 'wide masked' pixel
// ################################

#include "Fe65p2Xtalk3x3Scan.h"

Fe65p2Xtalk3x3Scan::Fe65p2Xtalk3x3Scan(Bookkeeper *k) : ScanBase(k) {

}

void Fe65p2Xtalk3x3Scan::init() {

    // Loop 1: Special combined mask / Qc loop
    std::shared_ptr<Fe65p2Xtalk3x3MaskLoop> maskStaging(new Fe65p2Xtalk3x3MaskLoop);

    // Loop 2: Trigger
    std::shared_ptr<Fe65p2TriggerLoop> triggerLoop(new Fe65p2TriggerLoop);

    // Loop 3: Data gatherer
    std::shared_ptr<StdDataLoop> dataLoop(new StdDataLoop);
    dataLoop->setVerbose(false);
    dataLoop->connect(g_data);

    this->addLoop(maskStaging);
    this->addLoop(triggerLoop);
    this->addLoop(dataLoop);
    
    engine.init();
}

void Fe65p2Xtalk3x3Scan::preScan() {

  g_fe65p2->setValue(&Fe65p2::PlsrDac, g_fe65p2->toVcal(2000));
  g_fe65p2->configDac();
  g_fe65p2->configurePixels();
    
  g_fe65p2->setLatency(60+5);
  g_fe65p2->setValue(&Fe65p2::TestHit, 0x0);
  g_fe65p2->enAnaInj();
  g_fe65p2->setValue(&Fe65p2::Latency, 60);
  g_fe65p2->configureGlobal();
  
  while(g_tx->isCmdEmpty() == 0);
}
