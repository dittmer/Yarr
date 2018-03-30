/*
 * Authors: T. Heim <timon.heim@cern.ch>
 * Date: 2016-Mar-22
 */

#include "Fe65p2XtalkMaskLoop.h"

Fe65p2XtalkMaskLoop::Fe65p2XtalkMaskLoop() : LoopActionBase() {
  m_mask = 0x0003; //For 2x1
  //m_mask = 0x000F; //For 2x2
    min = 0;
    max = 8;
    step = 1;
    m_cur =0;
    m_done = false;

    loopType = typeid(this);
}

void Fe65p2XtalkMaskLoop::init() {
    // TODO want to init mask here and then only shift it in execPart2()
    // Nothing to do
    m_cur = min;
    m_done = false;
}

void Fe65p2XtalkMaskLoop::end() {
    // Nothing to do
}

void Fe65p2XtalkMaskLoop::execPart1() {
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
    // All in parallel
    g_fe65p2->setValue(&Fe65p2::ColEn, 0xFFFF);
    g_fe65p2->setValue(&Fe65p2::ColSrEn, 0xFFFF);
    // Write to global regs
    g_fe65p2->configureGlobal();
    usleep(2000); // Wait for DAC 
    // Write mask to SR
    g_fe65p2->writePixel(m_mask<<(2*m_cur),m_mask<<(2*(m_cur^2))); //2x1
    /*
    if (m_cur < 4) g_fe65p2->writePixel(m_mask<<(4*m_cur),0x0,m_mask<<(4*(3-m_cur)),0x0); //2x2
    else g_fe65p2->writePixel(0x0,m_mask<<(4*(m_cur-4)),0x0,m_mask<<(4*(7-m_cur)));
    */      
    // Write to Pixel reg
    g_fe65p2->setValue(&Fe65p2::InjEnLd, 0x1);
    g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x3);
    g_fe65p2->configureGlobal();
    
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

void Fe65p2XtalkMaskLoop::execPart2() {
   m_cur += step;
   if (!(m_cur<max)) m_done = true;
}
