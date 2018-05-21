/*
 * Authors: T. Heim <timon.heim@cern.ch>,
 * Date: 2015-Jul-21
 */

#include "StdDataGatherer.h"

#include <chrono>
#include <thread>
#include <algorithm>

StdDataGatherer::StdDataGatherer() : LoopActionBase() {
    storage = NULL;
    loopType = typeid(this);
    min = 0;
    max = 0;
    step = 1;
    counter = 0;
}

void StdDataGatherer::init() {
    m_done = false;
    killswitch = false;
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;
}

void StdDataGatherer::end() {
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;

}

void StdDataGatherer::execPart1() {
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    if (g_tx->getTrigEnable() == 0)
        std::cerr << "### ERROR ### " << __PRETTY_FUNCTION__ << " : Trigger is not enabled, will get stuck here!" << std::endl;

}

sig_atomic_t signaled = 0;

void StdDataGatherer::execPart2() {
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    unsigned count = 0;
    uint32_t done = 0;
    uint32_t rate = 0;

    signaled = 0;
    signal(SIGINT, [](int signum){signaled = 1;});
    signal(SIGUSR1, [](int signum){signaled = 1;});

    std::cout << "### IMPORTANT ### Going into endless loop, interrupt with ^c (SIGINT)!" << std::endl;

    std::vector<RawData*> tmp_storage;
    RawData *newData = NULL;
    while (done == 0) {
        RawDataContainer *rdc = new RawDataContainer();
        rate = g_rx->getDataRate();
        if (verbose)
            std::cout << " --> Data Rate: " << rate/256.0/1024.0 << " MB/s" << std::endl;
        done = g_tx->isTrigDone();
        do {
            newData =  g_rx->readData();
            if (newData != NULL) {
                rdc->add(newData);
                count += newData->words;
            }
        } while (newData != NULL);
        delete newData;
        rdc->stat = *g_stat;
        storage->pushData(rdc);
        if (signaled == 1 || killswitch) {
            std::cout << "Caught interrupt, stopping data taking!" << std::endl;
            std::cout << "Abort will leave buffers full of data!" << std::endl;
            g_tx->toggleTrigAbort();
        }
        std::this_thread::sleep_for(std::chrono::microseconds(500));
    }
        
    m_done = true;
    counter++;
}

void StdDataGatherer::execPart2Standalone() {

    RawData *newData = NULL;
    RawDataContainer *rdc = new RawDataContainer();
    uint32_t done = g_tx->isTrigDone();
    if (done == 0){
      do {
	newData =  g_rx->readData();
	if (newData != NULL) {
	  rdc->add(newData);
	}
      } while (newData != NULL);
      delete newData;
      rdc->stat = *g_stat;
      storage->pushData(rdc);
      std::this_thread::sleep_for(std::chrono::microseconds(500));
      counter++;
    }
}

void StdDataGatherer::sendAbort(){
  g_tx->toggleTrigAbort();
}

void StdDataGatherer::connect(ClipBoard<RawDataContainer> *clipboard) {
    storage = clipboard;
}
