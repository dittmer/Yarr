/*
 * Authors: T. Heim <timon.heim@cern.ch>
 * Date: 2014-Sep-27
 */

#include "Fei4CrossTalkMaskLoop.h"

Fei4CrossTalkMaskLoop::Fei4CrossTalkMaskLoop() : LoopActionBase() {
    m_mask = MASK_16;
    min = 0;
    max = 16;
    step = 1;
    m_cur = 0;
    enable_sCap = false;
    enable_lCap = false;
    m_half = false;
    loopType = typeid(this);
}

void Fei4CrossTalkMaskLoop::enableAndInjectHalf(unsigned which){

    if (which == 1){

        for(unsigned i = 0; i < 21; i++) {
            if (i < 10 ) en_mask[i] = (m_mask) << m_cur;
            else if (i == 10) en_mask[i] = (m_mask & 0x0000FFFF);
            else en_mask[i] = 0x00000000;
        }

        g_fe->initMask(en_mask);
        g_fe->loadIntoPixel(1 << 0); //Enable readout in the enable mask

        for(unsigned i = 0; i < 21; i++) {
            inj_mask[i] = ~en_mask[i];
        }

    }else if (which == 2){

        for(unsigned i = 0; i < 21; i++) {
            if (i < 10) en_mask[i] = 0x00000000;
            else if (i == 10) en_mask[i] = (m_mask & 0xFFFF0000);
            else en_mask[i] = (m_mask) << m_cur;
        }

        g_fe->initMask(en_mask);
        g_fe->loadIntoPixel(1 << 0); //Enable readout in the enable mask

        for(unsigned i = 0; i < 21; i++) {
            inj_mask[i] = ~en_mask[i];
        }

    }

        //Inject into the injection mask
        g_fe->initMask(inj_mask);
        if (enable_lCap) g_fe->loadIntoPixel(1 << 6); //injection to the large capacitor.
        if (enable_sCap) g_fe->loadIntoPixel(1 << 7); //injection to the small capacitor.

}

void Fei4CrossTalkMaskLoop::init() {
    m_done = false;
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    // Shift Mask into all pixels
    g_fe->writeRegister(&Fei4::Colpr_Mode, 0x3); //writes into global register (write all dcs)
    g_fe->writeRegister(&Fei4::Colpr_Addr, 0x0); //read from column all columns ^ including from address 0
    //g_fe->initMask(MASK_1); //writes data from global reg to FE
    enableAndInjectHalf(1);
    m_cur = min;
    while(g_tx->isCmdEmpty() == 0);
}


void Fei4CrossTalkMaskLoop::end() {
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    // Disable all pixels
    g_fe->writeRegister(&Fei4::Colpr_Mode, 0x3);
    g_fe->writeRegister(&Fei4::Colpr_Addr, 0x0);
    g_fe->initMask(MASK_NONE);
    g_fe->loadIntoPixel(1 << 0);
    if (enable_lCap) g_fe->loadIntoPixel(1 << 6);
    if (enable_sCap) g_fe->loadIntoPixel(1 << 7);
    while(g_tx->isCmdEmpty() == 0);
}

void Fei4CrossTalkMaskLoop::execPart1() {
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;
//	std::cout << "Ingrid loves sweatpants" << std::endl;
    std::cout << " ---> Mask Stage " << m_cur << std::endl;
    g_stat->set(this, m_cur);
}

void Fei4CrossTalkMaskLoop::execPart2() {
    if (verbose)
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    m_cur += step;
    if ( m_cur == max && m_half == false){
        enableAndInjectHalf(2);
        m_cur = 0;
        m_half = true;
    }
    if (!(m_cur < max) && m_half == true) m_done = true;
    // Shift Enable mask by step size
    
}

void Fei4CrossTalkMaskLoop::setMaskStage(enum MASK_STAGE mask) {
    switch (mask) {
        case MASK_1:
            m_mask = MASK_1;
            max = 1;
            break;
        case MASK_2:
            m_mask = MASK_2;
            max = 2;
            break;
        case MASK_4:
            m_mask = MASK_4;
            max = 4;
            break;
        case MASK_8:
            m_mask = MASK_8;
            max = 8;
            break;
        case MASK_16:
            m_mask = MASK_16;
            max = 16;
            break;
        case MASK_32:
            m_mask = MASK_32;
            max = 32;
            break;
        case MASK_NONE:
            m_mask = MASK_NONE;
            max = 0;
            break;
    }
}
/*
void Fei4MaskLoop::setMaskStage(uint32_t mask) {
    m_mask = mask;
}

uint32_t Fei4MaskLoop::getMaskStage() {
    return m_mask;
}

void Fei4MaskLoop::setIterations(unsigned it) {
    m_it = it;
}

unsigned Fei4MaskLoop::getIterations() {
    return m_it;
}*/
