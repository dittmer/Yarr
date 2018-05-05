#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdio.h>

int main() {

  float avgToT[2]          = {};
  float avgToT50[2]        = {};
  float defaultHits[64*64] = {};
  float xtalkHits[64*64]   = {};
  float defaultSumHits = 0.0;
  float xtalkSumHits = 0.0;
  
  // Read nHits from normal file
  std::ifstream file1;
  file1.open("YarrPlots/XtalkResults/default/tmp_yarr_histo2d_fe65p2_ch0_totscan_OccMap.dat");
  if (!file1.is_open()) {
    std::cout << "Cannot open default nHits file" << std::endl;
    return 0;
  }

  std::string word;
  int counter = 0;
  while (file1 >> word){
    if (counter == 4096) {
      std::cout << "Error: too many numbers in default nHits file!" << std::endl;
      return 0;
    }
    defaultHits[counter] = std::stof(word);
    defaultSumHits += std::stof(word);
    counter++;
  }
  file1.close();

  std::cout << "Total hits in default file: " << defaultSumHits << std::endl;

  // Read nHits from xtalk file
  std::ifstream file2;
  file2.open("YarrPlots/XtalkResults/Xtalk2x2/tmp_yarr_histo2d_fe65p2_ch0_xtalkscan_OccMap.dat");
  if (!file2.is_open()) {
    std::cout << "Cannot open xtalk nHits file" << std::endl;
    return 0;
  }

  counter = 0;
  while (file2 >> word){
    if (counter == 4096) {
      std::cout << "Error: too many numbers in 1x1 file!" << std::endl;
      return 0;
    }
    xtalkHits[counter] = std::stof(word);
    xtalkSumHits += std::stof(word);
    counter++;
  }
  file2.close();

  std::cout << "Total hits in xtalk file: " << xtalkSumHits << std::endl;

  // Read Tot from default file
  std::ifstream file3;
  file3.open("YarrPlots/XtalkResults/default/tmp_yarr_histo2d_fe65p2_ch0_totscan_MeanTotMap.dat");
  if (!file3.is_open()) {
    std::cout << "Cannot open default ToT file" << std::endl;
    return 0;
  }

  float counter50 = 0.0;
  counter = 0;
  while (file3 >> word){
    if (counter == 4096) {
      std::cout << "Error: too many numbers in default ToT file!" << std::endl;
      return 0;
    }
    if ((defaultHits[counter] == 0) && (xtalkHits[counter] > 0)) {
      std::cout << "Inefficient pixel: " << defaultHits[counter] << " hits in default, " << xtalkHits[counter] << " hits in xtalk" << std::endl;
      std::cout << "Default ToT: " << word << std::endl;
      avgToT50[0] += std::stof(word);
      counter50 += 1.0;
    }
    avgToT[0] += std::stof(word);
    counter++;
  }
  file3.close();
  avgToT50[0] /= counter50;
  std::cout << "counter50 = " << counter50 << std::endl;
  counter50 = 0.0;
  
  // Read Tot from 1x1 file
  std::ifstream file4;
  file4.open("YarrPlots/XtalkResults/Xtalk2x2/tmp_yarr_histo2d_fe65p2_ch0_xtalkscan_MeanTotMap.dat");
  if (!file4.is_open()) {
    std::cout << "Cannot open xtalk ToT file" << std::endl;
    return 0;
  }

  counter = 0;
  while (file4 >> word){
    if (counter == 4096) {
      std::cout << "Error: too many numbers in xtalk ToT file!" << std::endl;
      return 0;
    }
    if (std::stof(word) == 14) std::cout << "Hit with ToT 14 number " << counter+1 << " row " << counter / 64 + 1 << " col " <<  counter % 64 + 1 << std::endl;
    if ((defaultHits[counter] == 0) && (xtalkHits[counter] > 0)) {
      std::cout << "Xtalk ToT: " << std::stof(word) << std::endl;
      avgToT50[1] += std::stof(word);
      counter50 += 1.0;
    }
    avgToT[1] += std::stof(word);
    counter++;
  }
  file4.close();
  avgToT50[1] /= counter50;
  std::cout << "counter50 = " << counter50 << std::endl;
  counter50 = 0.0;
  
  std::cout << "Average ToT:                           " << avgToT[0] / 4096.0 << " default " << avgToT[1] / 4096.0 << " Xtalk " << avgToT[1] / avgToT[0] << " ratio" << std::endl;
  std::cout << "Average ToT for pixels with 50 hits:   " << avgToT50[0] << " default " << avgToT50[1] << " Xtalk " << avgToT50[1] / avgToT50[0] << " ratio" << std::endl;
  
  return 0;
}

