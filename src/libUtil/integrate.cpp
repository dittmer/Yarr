#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdio.h>

int getMask (int counter){
  int column = counter % 4;
  int row = (counter / 64) % 4;
  int mask = -1;
  if (column == 0) mask = (row+2*(0+1*(row/2)))^1;
  if (column == 1) mask = (row+2*(1+1*(row/2)))^1;
  if (column == 2) mask = (row+2*(2-3*(row/2)))^1;
  if (column == 3) mask = (row+2*(3-3*(row/2)))^1;
  return mask;
}

int main() {

  float Tot_1x1[9][9] = {};
  float Tot_default[9][9] = {};
  float maskArray[64][64] = {};

  // Read Tot from normal file
  std::ifstream file1;
  file1.open("YarrPlots/XtalkResults/default/tmp_yarr_histo2d_fe65p2_ch0_totscan_MeanTotMap.dat");
  if (!file1.is_open()) {
    std::cout << "Cannot open default file" << std::endl;
    return 0;
  }

  std::string word;
  int counter = 0;
  while (file1 >> word){
    if (counter == 4096) {
      std::cout << "Error: too many numbers in default file!" << std::endl;
      return 0;
    }
    int column = counter % 64;
    int QC = (column / 4) % 8;
    int mask = getMask(counter);
    maskArray[counter / 64][column] = mask;
    Tot_default[QC][mask] += std::stof(word);
    Tot_default[QC][8] += std::stof(word);
    Tot_default[8][mask] += std::stof(word);
    counter++;
  }
  file1.close();

  // Read Tot from 1x1 file
  std::ifstream file2;
  file2.open("YarrPlots/XtalkResults/Tot_QCfirst/tmp_yarr_histo2d_fe65p2_ch0_totscan_MeanTotMap.dat");
  if (!file2.is_open()) {
    std::cout << "Cannot open 1x1 file" << std::endl;
    return 0;
  }

  counter = 0;
  while (file2 >> word){
    if (counter == 4096) {
      std::cout << "Error: too many numbers in 1x1 file!" << std::endl;
      return 0;
    }
    int column = counter % 64;
    int QC = (column / 4) % 8;
    int mask = getMask(counter);
    Tot_1x1[QC][mask] += std::stof(word);
    Tot_1x1[QC][8] += std::stof(word);
    Tot_1x1[8][mask] += std::stof(word);
    counter++;
  }
  file2.close();

  std::cout << "Test mask " << std::endl;
  for (int ii = 0; ii < 8; ii++){
    for (int jj = 0; jj < 8; jj++){
      std::cout << maskArray[7-ii][jj] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
  
  
  std::cout << "Average ToT per iteration " << std::endl;
  std::cout << "Default" << std::endl;
  std::cout << "Mask  1    2    3    4    5    6    7    8    Avg" << std::endl;
  std::cout << std::fixed << std::setprecision(2);
  for (int ii = 0; ii < 9; ii++){
    if (ii == 8) std::cout << "Avg   ";
    else std::cout << "QC" << ii << "   ";
    for (int jj = 0; jj < 9; jj++){
      if (ii == 8 && jj == 8) continue;
      std::cout << Tot_default[ii][jj] / ((ii == 8 || jj == 8) ? 64.0*8.0 : 64.0) << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;

  std::cout << "QCfirst" << std::endl;
  std::cout << "Mask  1    2    3    4    5    6    7    8    Avg" << std::endl;
  for (int ii = 0; ii < 9; ii++){
    if (ii == 8) std::cout << "Avg   ";
    else std::cout << "QC" << ii << "   ";
    for (int jj = 0; jj < 9; jj++){
      if (ii == 8 && jj == 8) continue;
      std::cout << Tot_1x1[ii][jj] / ((ii == 8 || jj == 8) ? 64.0*8.0 : 64.0) << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
  
  std::cout << "QCfirst / default" << std::endl;
  std::cout << "Mask  1    2    3    4    5    6    7    8    Avg" << std::endl;
  for (int ii = 0; ii < 9; ii++){
    if (ii == 8) std::cout << "Avg   ";
    else std::cout << "QC" << ii << "   ";
    for (int jj = 0; jj < 9; jj++){
      if (ii == 8 && jj == 8) continue;
      std::cout << Tot_1x1[ii][jj] / Tot_default[ii][jj] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
  
  return 0;
}

