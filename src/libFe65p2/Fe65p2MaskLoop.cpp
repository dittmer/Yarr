/*
 * Authors: T. Heim <timon.heim@cern.ch>
 * Date: 2016-Mar-22
 */

#include "Fe65p2MaskLoop.h"

Fe65p2MaskLoop::Fe65p2MaskLoop() : LoopActionBase() {
    m_mask = 0x0101;
    min = 0;
    max = 8;
    step = 1;
    m_cur =0;
    m_done = false;
    use_mask = true;

    loopType = typeid(this);
}

void Fe65p2MaskLoop::init() {
    // TODO want to init mask here and then only shift it in execPart2()
    // Nothing to do
    m_cur = min;
    m_done = false;
}

void Fe65p2MaskLoop::end() {
    // Nothing to do
}

void Fe65p2MaskLoop::execPart1() {
    std::cout << " ---> Mask Stage " << m_cur << std::endl;
    // TODO needs to be done per FE!
    // Set threshold high
    uint16_t tmp1 = g_fe65p2->getValue(&Fe65p2::Vthin1Dac);
    std::cout << tmp1 << std::endl;
    uint16_t tmp2 = g_fe65p2->getValue(&Fe65p2::Vthin2Dac);
    uint16_t tmp3 = g_fe65p2->getValue(&Fe65p2::VffDac);
    g_fe65p2->setValue(&Fe65p2::Vthin1Dac, 255);
    g_fe65p2->setValue(&Fe65p2::Vthin2Dac, 0);
    g_fe65p2->setValue(&Fe65p2::VffDac, 0);
    // Incorporating base mask from json
    if (use_mask) {
      //Turn all QC on
      g_fe65p2->setValue(&Fe65p2::ColEn, 0xFFFF);
      g_fe65p2->configureGlobal();
      usleep(2000);
      //Program one QC at a time
      for(unsigned i=0; i<Fe65p2Cfg::n_QC; i++) {
        setValue(&Fe65p2::ColSrEn, (0x1<<i));
        setValue(&Fe65p2::OneSr, 0);
        configureGlobal();
	//Get base InjEn mask (assumes same PixConf and InjEn masks)
	uint16_t* baseMask = g_fe65p2->getCfg(1,i);
	//Modify baseMask with mask stage
	for (unsigned j=0; j<Fe65p2PixelCfg::n_Words; j++){
	  baseMask[j] &= (m_mask<<m_cur);
	}
	writePixel(baseMask);
	setValue(&Fe65p2::SignLd, 0x1);
	setValue(&Fe65p2::PixConfLd, 0x3);
	configureGlobal();

	// Unset Shadow registers
	setValue(&Fe65p2::InjEnLd, 0x0);
	setValue(&Fe65p2::PixConfLd, 0x0);
	configureGlobal();
	writePixel((uint16_t) 0x0);
      }
    }
    else {
      // All in parallel
      g_fe65p2->setValue(&Fe65p2::ColEn, 0xFFFF);
      g_fe65p2->setValue(&Fe65p2::ColSrEn, 0xFFFF);
      // Write to global regs
      g_fe65p2->configureGlobal();
      usleep(2000); // Wait for DAC 
      // Write mask to SR
      g_fe65p2->writePixel((m_mask<<m_cur));
      // Write to Pixel reg
      g_fe65p2->setValue(&Fe65p2::InjEnLd, 0x1);
      g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x3);
      g_fe65p2->configureGlobal();
    }
    
    // Unset shadow reg and reset threshold
    g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x0);
    g_fe65p2->setValue(&Fe65p2::InjEnLd, 0x0);
    g_fe65p2->setValue(&Fe65p2::Vthin1Dac, tmp1);
    g_fe65p2->setValue(&Fe65p2::Vthin2Dac, tmp2);
    g_fe65p2->setValue(&Fe65p2::VffDac, tmp3);
    g_fe65p2->configureGlobal();
    usleep(5000); // Wait for DAC 

    // Leave SR set, as it enables the digital inj (if TestHit is set)
    while(g_tx->isCmdEmpty() == 0);
    g_stat->set(this, m_cur);
    
}

void Fe65p2MaskLoop::execPart2() {
   m_cur += step;
   if (!(m_cur<max)) m_done = true;
}
