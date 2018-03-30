// #################################
// # Author: Timon Heim
// # Email: timon.heim at cern.ch
// # Project: Yarr
// # Description: Fe65-P2 Noise Scan
// # Comment: Nothing fancy
// #################################

#include "Fe65p2NoiseScan.h"

Fe65p2NoiseScan::Fe65p2NoiseScan(Bookkeeper *k) : ScanBase(k) {
    triggerFrequency = 1e3;
    triggerTime = 360;
    verbose = false;
    start_fresh = true;
}

// Initialize Loops
void Fe65p2NoiseScan::init() {
    

    // Loop 1: Trigger
    std::shared_ptr<Fe65p2TriggerLoop> triggerLoop(new Fe65p2TriggerLoop);
    triggerLoop->setVerbose(verbose);
    triggerLoop->setTrigFreq(triggerFrequency);
    triggerLoop->setTrigTime(triggerTime);
    triggerLoop->setTrigCnt(0); // Activated time mode
    triggerLoop->setNoInject();
    //triggerLoop->setExtTrigger();

    // Loop 2: Data gatherer
    std::shared_ptr<StdDataGatherer> dataLoop(new StdDataGatherer);
    dataLoop->setVerbose(verbose);
    dataLoop->connect(g_data);

    this->addLoop(triggerLoop);
    this->addLoop(dataLoop);
    
    engine.init();
}

// Do necessary pre-scan configuration
void Fe65p2NoiseScan::preScan() {
  g_fe65p2->configurePixels();
  g_fe65p2->setValue(&Fe65p2::TrigCount, 10);
  g_fe65p2->setValue(&Fe65p2::Latency, 70);
  g_fe65p2->configureGlobal();
  if (start_fresh) {
    g_fe65p2->setValue(&Fe65p2::ColEn, 0xFFFF);
    g_fe65p2->setValue(&Fe65p2::ColSrEn, 0xFFFF);
    g_fe65p2->configureGlobal();
    usleep(2000);
    g_fe65p2->writePixel((uint16_t)0xffff);
    g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x3);
    g_fe65p2->configureGlobal();
    g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x0);
    g_fe65p2->configureGlobal();
    g_fe65p2->writePixel((uint16_t)0x0);
  }
  while(!g_tx->isCmdEmpty());
}

