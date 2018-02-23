/*
 * Authors: T. Heim <timon.heim@cern.ch>
 * Date: 2016-Mar-22
 */

#include "Fe65p2XtalkMaskLoop.h"

Fe65p2XtalkMaskLoop::Fe65p2XtalkMaskLoop() : LoopActionBase() {
    min = 0;
    max = 32; //64 col masks x 4 (modular) row masks
    step = 1;
    m_cur = 0;
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
  
  // Loop is first over columns, then rows
  unsigned global_col = m_cur / 4;                 // Column # in chip
  unsigned i_QC       = global_col / 4;            // QC containing column
  unsigned local_col  = global_col % 4;            // Column # in QC
  unsigned local_row  = m_cur % 4;                 // Row # modulo 4 (inject every 4)

  int shift = getShift(global_col);

  // Get ready to program QCs
  uint16_t tmp1 = g_fe65p2->getValue(&Fe65p2::Vthin1Dac);
  uint16_t tmp2 = g_fe65p2->getValue(&Fe65p2::Vthin2Dac);
  uint16_t tmp3 = g_fe65p2->getValue(&Fe65p2::VffDac);
  g_fe65p2->setValue(&Fe65p2::Vthin1Dac, 255);
  g_fe65p2->setValue(&Fe65p2::Vthin2Dac, 0);
  g_fe65p2->setValue(&Fe65p2::VffDac, 0);
  
  // Select primary QC
  g_fe65p2->setValue(&Fe65p2::ColSrEn, (0x1<<i_QC)); //ColSrEn used to program columns
  g_fe65p2->setValue(&Fe65p2::OneSr, 0);
  g_fe65p2->configureGlobal();

  // Build InjEn mask
  std::pair<uint16_t,uint16_t> m_InjEn = getMaskInj(local_col,local_row);

  // Program InjEn mask
  g_fe65p2->writePixel(m_InjEn.first, m_InjEn.second);
  g_fe65p2->setValue(&Fe65p2::InjEnLd, 0x1);
  g_fe65p2->configureGlobal();
  g_fe65p2->setValue(&Fe65p2::InjEnLd, 0x0);
  g_fe65p2->configureGlobal();
  g_fe65p2->writePixel((uint16_t)0x0);

  // Build PixConf mask
  std::pair<uint16_t, uint16_t> m_PixConf = getMaskEn(local_col,local_row);

  // Program PixConf mask
  g_fe65p2->writePixel(m_PixConf.first, m_PixConf.second);
  g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x3);
  g_fe65p2->configureGlobal();
  g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x0);
  g_fe65p2->configureGlobal();
  g_fe65p2->writePixel((uint16_t)0x0);

  // Program PixConf mask for secondary QC if needed
  if (shift != 0){
    g_fe65p2->setValue(&Fe65p2::ColSrEn, (0x1<<(i_QC+shift)));
    g_fe65p2->setValue(&Fe65p2::OneSr, 0);
    g_fe65p2->configureGlobal();
    
    m_PixConf = getMaskEn(local_col,local_row,shift);

    g_fe65p2->writePixel(m_PixConf.first, m_PixConf.second);
    g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x3);
    g_fe65p2->configureGlobal();
    g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x0);
    g_fe65p2->configureGlobal();
    g_fe65p2->writePixel((uint16_t)0x0);
  }

  // Done with configuring, set thresholds / feedback to operating values
  g_fe65p2->setValue(&Fe65p2::Vthin1Dac, tmp1);
  g_fe65p2->setValue(&Fe65p2::Vthin2Dac, tmp2);
  g_fe65p2->setValue(&Fe65p2::VffDac, tmp3);
  g_fe65p2->configureGlobal();
  usleep(5000); // Wait for DAC

  // TODO: should this only be set for cols we are injecting later?
  g_fe65p2->setValue(&Fe65p2::ColSrEn, 0xFFFF);
  
  while(g_tx->isCmdEmpty() == 0);
  g_stat->set(this, m_cur); //TODO: this might need to be changed
  
  // Enable relevant QC(s)
  g_fe65p2->setValue(&Fe65p2::ColEn, ((0x1 << i_QC) | (0x1 << (i_QC+shift))));
  g_fe65p2->configureGlobal();
   
}

void Fe65p2XtalkMaskLoop::execPart2() {
   m_cur += step;

   g_fe65p2->setValue(&Fe65p2::ColSrEn, 0xFFFF);
   g_fe65p2->setValue(&Fe65p2::OneSr, 0);
   g_fe65p2->configureGlobal();
   usleep(2000);
   g_fe65p2->writePixel((uint16_t)0x0);
   g_fe65p2->setValue(&Fe65p2::InjEnLd, 0x1);
   g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x3);
   g_fe65p2->configureGlobal();
   g_fe65p2->setValue(&Fe65p2::PixConfLd, 0x0);
   g_fe65p2->setValue(&Fe65p2::InjEnLd, 0x0);
   g_fe65p2->configureGlobal();

   if (!(m_cur<max)) m_done = true;
}

// Get injection mask words
std::pair<uint16_t, uint16_t> Fe65p2XtalkMaskLoop::getMaskInj(unsigned col, unsigned row){
  std::pair<uint16_t, uint16_t> m_out;
  m_out.first  = getColMask(col,1) & getRowMask(row,1);
  m_out.second = getColMask(col,2) & getRowMask(row,2);
  return m_out;
}

// Get enable mask words per QC(s)
std::pair<uint16_t, uint16_t> Fe65p2XtalkMaskLoop::getMaskEn(unsigned col, unsigned row, int shift){
  std::pair<uint16_t, uint16_t> m_out;
  unsigned col_wrap = col-4*shift; 
  m_out.first  = (getColMask(col_wrap-1,1) | getColMask(col_wrap,1) | getColMask(col_wrap+1,1)) & (getRowMask(row-1,1) | getRowMask(row,1) | getRowMask(row+1,1));
  m_out.second = (getColMask(col_wrap-1,2) | getColMask(col_wrap,2) | getColMask(col_wrap+1,2)) & (getRowMask(row-1,2) | getRowMask(row,2) | getRowMask(row+1,2));
  return m_out;
}

uint16_t Fe65p2XtalkMaskLoop::getRowMask(unsigned row, unsigned word){
  if (row >= 0 && row < 4) {
    if (word == 1) return m1_row[row];
    else return m2_row[row];
  }
  else return 0x00;
};

uint16_t Fe65p2XtalkMaskLoop::getColMask(unsigned col, unsigned word){
  if (col >= 0 && col < 4) {
    if (word == 1) return m1_col[col];
    else return m2_col[col];
  }
  else return 0x00;
};

int Fe65p2XtalkMaskLoop::getShift(unsigned global_col){
  if (global_col == 0 || global_col >= 64) return 0;
  else if (global_col % 4 == 0) return -1;
  else if (global_col % 4 == 3) return +1;
  else return 0;
};
