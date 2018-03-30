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
      // Power all QCs
      g_fe65p2->setValue(&Fe65p2::ColEn, 0xFFFF);
      g_fe65p2->configureGlobal();
      for(unsigned i=0; i<Fe65p2Cfg::n_QC; i++) {
	// Enable one QC at a time
	g_fe65p2->setValue(&Fe65p2::ColSrEn, (0x1)<<i);
	//g_fe65p2->setValue(&Fe65p2::OneSr, 0);
	g_fe65p2->configureGlobal();
	usleep(2000); // Wait for DAC
	// Get mask from JSON
	uint16_t* baseMask = g_fe65p2->getCfg(6,i);
	uint16_t newMask[Fe65p2PixelCfg::n_Words] = {};
	//Add mask stage on top of base mask
	for (unsigned j=0; j<Fe65p2PixelCfg::n_Words; j++){
	  newMask[j] = baseMask[j] & (m_mask<<m_cur);
	}
	// Write mask to SR
	g_fe65p2->writePixel(newMask);
	// Write to Pixel reg
	g_fe65p2->setValue(&Fe65p2::InjEnLd, 0x1);
	g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x3);
	g_fe65p2->configureGlobal();
	// Clear registers when done
	g_fe65p2->setValue(&Fe65p2::InjEnLd, 0x0);
	g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x0);
	g_fe65p2->configureGlobal();
	usleep(2000);
      }
      // Apparently ColSrEn also needs to be on to read out QC? Set on for all
      g_fe65p2->setValue(&Fe65p2::ColSrEn, 0xFFFF);
      g_fe65p2->configureGlobal();
      usleep(2000);
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
