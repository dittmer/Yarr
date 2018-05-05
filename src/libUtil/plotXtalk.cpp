#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

int main(int argc, char *argv[]) {

  float Val1[4096];
  float Val2[4096];

  if (argc != 4) {
    std::cout << "Wrong number of arguments! Should be ./plotXtalk sample1 sample2 quantity" << std::endl;
    return 0;
  }

  std::string sample1 = argv[1];
  std::string sample2 = argv[2];
  std::string quantity = argv[3];

  std::string type1 = (sample1.find("Xtalk") != std::string::npos) ? "xtalk" : "tot";
  std::string type2 = (sample2.find("Xtalk") != std::string::npos) ? "xtalk" : "tot";

  // Read Tot from normal file
  std::ifstream file1;
  std::string filename1 = "YarrPlots/XtalkResults/"+sample1+"/tmp_yarr_histo2d_fe65p2_ch0_"+type1+"scan_"+quantity+".dat";
  file1.open(filename1);
  if (!file1.is_open()) {
    std::cout << "Cannot open " << filename1 << std::endl;
    return 0;
  }

  std::string word;
  int counter = 0;
  while (file1 >> word){
    if (counter < 4096) Val1[counter] = std::stof(word);
    counter++;
  }
  if (counter > 4096) {
    std::cout << "Error: too many numbers in " << sample1 << " file! There are " << counter << " numbers." << std::endl;
    return 0;
  }
  
  file1.close();
  
  // Read Tot file for 3x3 mask
  std::ifstream file2;
  std::string filename2 = "YarrPlots/XtalkResults/"+sample2+"/tmp_yarr_histo2d_fe65p2_ch0_"+type2+"scan_"+quantity+".dat";
  file2.open(filename2);
  if (!file2.is_open()) {
    std::cout << "Cannot open " << filename2 << std::endl;
    return 0;
  }

  counter = 0;
  while (file2 >> word){
    if (counter < 4096) Val2[counter] = std::stof(word);
    counter++;
  }
  if (counter > 4096) {
    std::cout << "Error: too many numbers in " << sample2 << " file! There are " << counter << " numbers." << std::endl;
    return 0;
  }

  file2.close();

  //Get difference
  std::fstream fileout("DiffTotMap.dat",std::fstream::out | std::fstream::trunc);
  for (int ii = 0; ii < 4096; ii++){
    if (quantity == "OccMap") fileout << (Val2[ii] - Val1[ii]) << " ";
    if (quantity == "MeanTotMap") {
      if (Val1[ii] == 0.0 && Val2[ii] == 0.0) fileout << "0.0 ";
      else if (Val1[ii] == 0.0) fileout << "10.0 "; //Will be out of range
      else fileout << (Val2[ii] - Val1[ii]) / Val1[ii] << " ";
    }
    if (ii % 64 == 63) fileout << std::endl;
  }
  fileout.close();

  // Do plotting
  std::string finalname;
  if (quantity == "OccMap") finalname = "nHits";
  if (quantity == "MeanTotMap") finalname = "ToT";
  std::string cmd = "gnuplot > YarrPlots/XtalkResults/Diff"+finalname+"Map_"+sample2+"vs"+sample1+".png";

  // Open gnuplot as file and pipe commands
  FILE *gnu = popen(cmd.c_str(), "w");
    
  fprintf(gnu, "set terminal png size 1280, 1024 font \"Helvetica,22\"\n");
  fprintf(gnu, "set palette negative defined ( 0 '#D53E4F', 1 '#F46D43', 2 '#FDAE61', 3 '#FEE08B', 4 '#E6F598', 5 '#ABDDA4', 6 '#66C2A5', 7 '#3288BD')\n");
  fprintf(gnu, "unset key\n");
  fprintf(gnu, "set title \"Difference in %s when adding cross talk\"\n", finalname.c_str());
  fprintf(gnu, "set xlabel \"Cols\"\n");
  fprintf(gnu, "set ylabel \"Rows\"\n");
  if (quantity == "OccMap") {
    fprintf(gnu, "set cblabel \"nHits[%s] - nHits[%s]\"\n", sample2.c_str(), sample1.c_str());
    fprintf(gnu, "set cbrange [-4.0:10.0]\n");
  }
  if (quantity == "MeanTotMap") {
    fprintf(gnu, "set cblabel \"(ToT[%s] - ToT[%s]) / ToT[%s]\"\n", sample1.c_str(), sample1.c_str(), sample1.c_str());
    fprintf(gnu, "set cbrange [-0.3:0.3]\n");
  }
  fprintf(gnu, "set xrange [0.5:64.5]\n");
  fprintf(gnu, "set yrange [0.5:64.5]\n");
  //fprintf(gnu, "set zrange [-1.0:1.0]\n");
  fprintf(gnu, "plot \"DiffTotMap.dat\" matrix u (($1)*((%f-%f)/%d.0)+%f):(($2)*((%f-%f)/%d.0)+%f):3 with image\n", 64.5, 0.5, 64, 1.0, 64.5, 0.5, 64, 1.0);
  pclose(gnu);
  
  return 0;
}
