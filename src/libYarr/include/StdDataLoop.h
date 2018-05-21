/*
 * Authors: T. Heim <timon.heim@cern.ch>,
 * Date: 2014-Sep-27
 */

#ifndef STDDATALOOP_H
#define STDDATALOOP_H

#include "LoopActionBase.h"
#include "ClipBoard.h"
#include "RawData.h"
#include <signal.h>

class StdDataLoop: public LoopActionBase {
    public:
        StdDataLoop();
        void connect(ClipBoard<RawDataContainer> *clipboard);
        void init();
        void end();
        void execPart1();
        void execPart2();
	void execPart2Standalone();
	void sendAbort();
	void pushData();
	void kill() {
		killswitch = true;
	}
        
    private:
        ClipBoard<RawDataContainer> *storage;
        unsigned counter;
	bool killswitch;
	RawDataContainer* rdc_global;
};

#endif


