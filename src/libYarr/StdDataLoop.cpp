/*
 * Authors: T. Heim <timon.heim@cern.ch>,
 * Date: 2014-Sep-27
 */

#include "StdDataLoop.h"

#include <chrono>
#include <thread>
#include <algorithm>

StdDataLoop::StdDataLoop() : LoopActionBase() {
    storage = NULL;
    loopType = typeid(this);
    min = 0;
    max = 0;
    step = 1;
    counter = 0;
}

void StdDataLoop::init() {
    m_done = false;
    killswitch = false;
    rdc_global = new RawDataContainer();
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;
}

void StdDataLoop::end() {
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;

}

void StdDataLoop::execPart1() {
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    if (g_tx->getTrigEnable() == 0)
        std::cerr << "### ERROR ### " << __PRETTY_FUNCTION__ << " : Trigger is not enabled, will get stuck here!" << std::endl;

}

sig_atomic_t signaled2 = 0;

void StdDataLoop::execPart2() {
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    unsigned count = 0;
    uint32_t done = 0;
    //uint32_t rate = 0;
    //uint32_t curCnt = 0;
    unsigned iterations = 0;
    //uint32_t startAddr = 0;

    signaled2 = 0;
    signal(SIGINT, [](int signum){signaled2 = 1;});
    signal(SIGUSR1, [](int signum){signaled2 = 1;});

    std::vector<RawData*> tmp_storage;
    RawData *newData = NULL;
    RawDataContainer *rdc = new RawDataContainer();
    while (done == 0) {
        //rate = g_rx->getDataRate();
        //curCnt = g_rx->getCurCount();
        done = g_tx->isTrigDone();
        do {
            newData =  g_rx->readData();
            iterations++;
            if (newData != NULL) {
                count += newData->words;
                rdc->add(newData);
            }
        } while (newData != NULL);
        //delete newData;
        if (signaled2 == 1 || killswitch) {
            std::cout << "Caught interrupt, stopping data taking!" << std::endl;
            std::cout << "Abort will leave buffers full of data!" << std::endl;
            g_tx->toggleTrigAbort();
        }
    }
    // Gather rest of data after timeout (~0.1ms)
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    do {
        //curCnt = g_rx->getCurCount();
        newData =  g_rx->readData();
        iterations++;
        if (newData != NULL) {
            count += newData->words;
            rdc->add(newData);
        }
    } while (newData != NULL);
    delete newData;
    
    rdc->stat = *g_stat;
    storage->pushData(rdc);
        
    if (verbose)
        std::cout << " --> Received " << count << " words! " << iterations << std::endl;
    m_done = true;
    counter++;
}

void StdDataLoop::execPart2Standalone() {
	
    RawData *newData = NULL;
    
    uint32_t done = 0 /*g_tx->isTrigDone()*/;
    if (done == 0) {
        do {
            newData =  g_rx->readData();
            if (newData != NULL) {
                rdc_global->add(newData);
            }
        } while (newData != NULL);
        //delete newData;
    }
    else
	m_done = true;
    // Gather rest of data after timeout (~0.1ms)
    //std::this_thread::sleep_for(std::chrono::microseconds(500));
    delete newData;
            
}

void StdDataLoop::sendAbort() {
	g_tx->toggleTrigAbort();
	std::this_thread::sleep_for(std::chrono::microseconds(500));
}

void StdDataLoop::pushData() {
    rdc_global->stat = *g_stat;
    storage->pushData(rdc_global);
    counter++;
}

void StdDataLoop::connect(ClipBoard<RawDataContainer> *clipboard) {
    storage = clipboard;
}
