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

    private:
        unsigned m_cur;
	uint16_t m1_col[4] = {0x0000,0x0000,0x3333,0xcccc};
	uint16_t m2_col[4] = {0x3333,0xcccc,0x0000,0x0000};
	uint16_t m1_row[4] = {0xa0a0,0x5050,0x0a0a,0x0505};
	uint16_t m2_row[4] = {0x0a0a,0x0505,0xa0a0,0x5050};
	
	uint16_t getRowMask(unsigned row, unsigned word);
	uint16_t getColMask(unsigned col, unsigned word);
	int getShift(unsigned global_col);

        void init();
        void end();
        void execPart1();
        void execPart2();

	std::pair<uint16_t, uint16_t> getMaskInj(unsigned col, unsigned row);
	std::pair<uint16_t, uint16_t> getMaskEn(unsigned col, unsigned row, int shift = 0);

};
#endif
