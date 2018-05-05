#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>

int main() {
  std::string numbers_to_integrate = "25458 624 726 520 391 365 254 163 102 73 58 31 21 8 4 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 2 4 13 29 44 71 124 187 298 416 413 459 535 358 215 31 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0";
  std::istringstream buf(numbers_to_integrate);
  std::string word;
  int counter = 0;
  float sum = 0.0;
  while (buf >> word){
    sum += std::stof(word) * counter;
    counter++;
  }
  std::cout << "Integral is " << sum << std::endl;
  return 0;
}
