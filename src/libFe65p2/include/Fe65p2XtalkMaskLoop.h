#ifndef FE65P2XTALKMASKLOOP_H
#define FE65P2XTALKMASKLOOP_H

/*
 * Authors: T. Heim <timon.heim@cern.ch>
 * Date: 2016-Mar-22
 */

#include <iostream>

#include "LoopActionBase.h"

class Fe65p2XtalkMaskLoop : public LoopActionBase {
    public:
        Fe65p2XtalkMaskLoop();
        void setMaskStage(uint16_t mask);

    private:
        uint16_t m_mask;
        unsigned m_cur;

        void init();
        void end();
        void execPart1();
        void execPart2();
};
#endif
