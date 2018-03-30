#ifndef FE65P2MASKLOOP_H
#define FE65P2MASKLOOP_H

/*
 * Authors: T. Heim <timon.heim@cern.ch>
 * Date: 2016-Mar-22
 */

#include <iostream>

#include "LoopActionBase.h"

class Fe65p2MaskLoop : public LoopActionBase {
    public:
        Fe65p2MaskLoop();
        void setMaskStage(uint16_t mask);

    private:
        uint16_t m_mask;
        unsigned m_cur;
	bool use_mask;

        void init();
        void end();
        void execPart1();
        void execPart2();
};
#endif
