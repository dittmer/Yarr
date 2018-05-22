#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <iomanip>
#include <cctype> //w'space detection
#include <ctime>
#include <map>
#include <sstream>

#include "HwController.h"

#include "AllHwControllers.h"
#include "AllChips.h"
#include "AllProcessors.h"

#include "Bookkeeper.h"
#include "Fei4.h"
#include "ScanBase.h"
#include "ScanFactory.h"
#include "Fei4DataProcessor.h"
#include "Fei4Histogrammer.h"
#include "Fei4Analysis.h"

//========================================================================================================================
std::unique_ptr<ScanBase> buildScan( const std::string& scanType, Bookkeeper& bookie );

void buildHistogrammers( std::map<FrontEnd*, std::unique_ptr<DataProcessor>>& histogrammers, const std::string& scanType, std::vector<FrontEnd*>& feList, ScanBase* s, std::string outputDir);

void buildAnalyses( std::map<FrontEnd*, std::unique_ptr<DataProcessor>>& analyses, const std::string& scanType, Bookkeeper& bookie, ScanBase* s, int mask_opt);

//========================================================================================================================

int main(){
  std::string ctrlCfgPath = "/home/pixeluser/Yarr/src/configs/controller/specCfg.json";
  std::string feCfgPath = "/home/pixeluser/Yarr/src/configs/connectivity/example_rd53a_setup.json";
  std::string scanType = "/home/pixeluser/Yarr/src/configs/scans/rd53a/std_noisescan.json";

  std::vector<std::string> cConfigPaths;
  std::string strippedScan;
  std::unique_ptr<HwController> hwCtrl;
  std::unique_ptr<Bookkeeper> bookie_ptr;
  std::map<FrontEnd*, std::string> feCfgMap;  
  std::unique_ptr<ScanBase> s;
  std::shared_ptr<DataProcessor> proc;
  std::map<FrontEnd*, std::unique_ptr<DataProcessor> > histogrammers;
  std::map<FrontEnd*, std::unique_ptr<DataProcessor> > analyses;

  std::cout << "\033[1;31m#####################################\033[0m" << std::endl;
  std::cout << "\033[1;31m# Welcome to the YARR Scan Console! #\033[0m" << std::endl;
  std::cout << "\033[1;31m#####################################\033[0m" << std::endl;
  
  std::cout << "\033[1;-> Parsing command line parameters ..." << std::endl;
  
  // Init parameters
  //std::string scanType = "";    
  std::string outputDir = "./data/";
  //std::string ctrlCfgPath = "";
  bool doPlots = false;
  int target_charge = -1;
  int target_tot = -1;
  int mask_opt = 0; //DISABLE MASKING BY DEFAULT
    
  unsigned runCounter = 0;

  // Load run counter
  if (system("mkdir -p ~/.yarr") < 0) {
    std::cerr << "#ERROR# Loading run counter ~/.yarr!" << std::endl;
  }
  
    std::string home = getenv("HOME");
    std::fstream iF((home + "/.yarr/runCounter").c_str(), std::ios::in);
    if (iF) {
        iF >> runCounter;
        runCounter += 1;
    } else {
        if (system("echo \"1\n\" > ~/.yarr/runCounter") < 0) {
            std::cerr << "#ERROR# trying to run echo!" << std::endl;
        }
        runCounter = 1;
    }
    iF.close();

    std::fstream oF((home + "/.yarr/runCounter").c_str(), std::ios::out);
    oF << runCounter << std::endl;
    oF.close();

    cConfigPaths.push_back(feCfgPath);
    doPlots = true;

    if (cConfigPaths.size() == 0) {
      throw std::runtime_error("Error: no config files given, please specify config file name under -c option, even if file does not exist!");
    }

    std::size_t pathPos = scanType.find_last_of('/');
    std::size_t suffixPos = scanType.find_last_of('.');
    if (pathPos != std::string::npos && suffixPos != std::string::npos) {
        strippedScan = scanType.substr(pathPos+1, suffixPos-pathPos-1);
    } else {
        strippedScan = scanType;
    }

    //outputDir += (toString(runCounter, 6) + "_" + strippedScan + "/");
    outputDir += (std::to_string(runCounter) + "_" + strippedScan + "/");

    std::cout << " Scan Type/Config: " << scanType << std::endl;
    
    std::cout << " Connectivity: " << std::endl;
    for(std::string const& sTmp : cConfigPaths){
        std::cout << "    " << sTmp << std::endl;
    }
    std::cout << " Target ToT: " << target_tot << std::endl;
    std::cout << " Target Charge: " << target_charge << std::endl;
    std::cout << " Output Plots: " << doPlots << std::endl;
    std::cout << " Output Directory: " << outputDir << std::endl;

    std::cout << std::endl;
  std::cout << "\033[1;31m#################\033[0m" << std::endl;
  std::cout << "\033[1;31m# Init Hardware #\033[0m" << std::endl;
  std::cout << "\033[1;31m#################\033[0m" << std::endl;
  
  if (ctrlCfgPath == "") {
    throw std::runtime_error("#ERRROR# No controller config given, aborting.");
  } else {
    // Open controller config file
    std::cout << "-> Opening controller config: " << ctrlCfgPath << std::endl;
    std::ifstream ctrlCfgFile(ctrlCfgPath);
    if (!ctrlCfgFile) {
      std::stringstream err;
      err << "#ERROR# Cannot open controller config file: " << ctrlCfgPath;
      throw std::runtime_error(err.str());
    }
    json ctrlCfg;
    try {
      ctrlCfg = json::parse(ctrlCfgFile);
    } catch (json::parse_error &e) {
      throw e;
    }
    std::string controller = ctrlCfg["ctrlCfg"]["type"];
    
    hwCtrl = StdDict::getHwController(controller);
    
    if(hwCtrl) {
      std::cout << "-> Found config for controller " << controller << std::endl;
      
      hwCtrl->loadConfig(ctrlCfg["ctrlCfg"]["cfg"]);
    } else {
      std::cerr << "#ERROR# Unknown config type: " << ctrlCfg["ctrlCfg"]["type"] << std::endl;
      std::cout << " Known HW controllers:\n";
      for(auto &h: StdDict::listHwControllers()) {
	std::cout << "  " << h << std::endl;
      }
      throw std::runtime_error("Aborting!");
    }
  }
  
  // Disable trigger in-case
  hwCtrl->setTrigEnable(0);
  
  bookie_ptr.reset(new Bookkeeper(&*hwCtrl, &*hwCtrl));

  bookie_ptr->setTargetTot(target_tot);
  bookie_ptr->setTargetCharge(target_charge);
  
  std::cout << "\033[1;31m#######################\033[0m" << std::endl
	    << "\033[1;31m##  Loading Configs  ##\033[0m" << std::endl
	    << "\033[1;31m#######################\033[0m" << std::endl;
  
  int success = 0;
  std::string chipType;
  
  // Loop over setup files
  for(std::string const& sTmp : cConfigPaths){
    std::cout << "Opening global config: " << sTmp << std::endl;
    std::ifstream gConfig(sTmp);
    json config;
    try {
      config = json::parse(gConfig);
    } catch(json::parse_error &e) {
      std::cerr << __PRETTY_FUNCTION__ << " : " << e.what() << std::endl;
    }
    
    if (config["chipType"].empty() || config["chips"].empty()) {
      std::stringstream err;
      err << __PRETTY_FUNCTION__ << " : invalid config, chip type or chips not specified!" << std::endl;
      throw std::runtime_error(err.str());
    } else {
      chipType = config["chipType"];
      std::cout << "Chip Type: " << chipType << std::endl;
      std::cout << "Found " << config["chips"].size() << " chips defined!" << std::endl;
      // Loop over chips
      for (unsigned i=0; i<config["chips"].size(); i++) {
	std::cout << "Loading chip #" << i << std::endl;
	try { 
	  json chip = config["chips"][i];
	  std::string chipConfigPath = chip["config"];
	  // TODO should be a shared pointer
	  bookie_ptr->addFe(StdDict::getFrontEnd(chipType).release(), chip["tx"], chip["rx"]);
	  bookie_ptr->getLastFe()->init(&*hwCtrl, chip["tx"], chip["rx"]);
	  std::ifstream cfgFile(chipConfigPath);
	  if (cfgFile) {
	    // Load config
	    std::cout << "Loading config file: " << chipConfigPath << std::endl;
	    json cfg = json::parse(cfgFile);
	    dynamic_cast<FrontEndCfg*>(bookie_ptr->getLastFe())->fromFileJson(cfg);
	    cfgFile.close();
	  } else {
	    std::cout << "Config file not found, using default!" << std::endl;
	  }
	  success++;
	  // Save path to config
	  std::size_t botDirPos = chipConfigPath.find_last_of("/");
	  feCfgMap[bookie_ptr->getLastFe()] = chipConfigPath;
	  dynamic_cast<FrontEndCfg*>(bookie_ptr->getLastFe())->setConfigFile(chipConfigPath.substr(botDirPos, chipConfigPath.length()));
          
	  // Create backup of current config
	  // TODO fix folder
	  std::ofstream backupCfgFile(outputDir + dynamic_cast<FrontEndCfg*>(bookie_ptr->getLastFe())->getConfigFile() + ".before");
	  json backupCfg;
	  dynamic_cast<FrontEndCfg*>(bookie_ptr->getLastFe())->toFileJson(backupCfg);
	  backupCfgFile << std::setw(4) << backupCfg;
	  backupCfgFile.close();
          
	} catch (json::parse_error &e) {
	  std::cerr << __PRETTY_FUNCTION__ << " : " << e.what() << std::endl;
	}
      }
    }
  }
  
    
  bookie_ptr->initGlobalFe(StdDict::getFrontEnd(chipType).release());
  bookie_ptr->getGlobalFe()->makeGlobal();
  bookie_ptr->getGlobalFe()->init(&*hwCtrl, 0, 0);
  
  std::cout << std::endl;
  std::cout << "\033[1;31m#################\033[0m" << std::endl;
  std::cout << "\033[1;31m# Configure FEs #\033[0m" << std::endl;
  std::cout << "\033[1;31m#################\033[0m" << std::endl;
  
  std::chrono::steady_clock::time_point cfg_start = std::chrono::steady_clock::now();
  for ( FrontEnd* fe : bookie_ptr->feList ) {
    std::cout << "-> Configuring " << dynamic_cast<FrontEndCfg*>(fe)->getName() << std::endl;
    // Select correct channel
    hwCtrl->setCmdEnable(0x1 << dynamic_cast<FrontEndCfg*>(fe)->getTxChannel());
    // Configure
    fe->configure();
    // Wait for fifo to be empty
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    while(!hwCtrl->isCmdEmpty());
  }
  std::chrono::steady_clock::time_point cfg_end = std::chrono::steady_clock::now();
  std::cout << "-> All FEs configured in " 
	    << std::chrono::duration_cast<std::chrono::milliseconds>(cfg_end-cfg_start).count() << " ms !" << std::endl;
  
  // Wait for rx to sync with FE stream
  // TODO Check RX sync
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
  // Enable all active channels
  hwCtrl->setCmdEnable(bookie_ptr->getTxMask());
  std::cout << "-> Setting Tx Mask to: 0x" << std::hex << bookie_ptr->getTxMask() << std::dec << std::endl;
  hwCtrl->setRxEnable(bookie_ptr->getRxMask());
  std::cout << "-> Setting Rx Mask to: 0x" << std::hex << bookie_ptr->getRxMask() << std::dec << std::endl;
  
  std::cout << std::endl;
  std::cout << "\033[1;31m##############\033[0m" << std::endl;
  std::cout << "\033[1;31m# Setup Scan #\033[0m" << std::endl;
  std::cout << "\033[1;31m##############\033[0m" << std::endl;
  
  // TODO Make this nice
  s = buildScan(scanType, *bookie_ptr );
    
  // TODO not to use the raw pointer!
  buildHistogrammers( histogrammers, scanType, bookie_ptr->feList, s.get(), outputDir);
  buildAnalyses( analyses, scanType, *bookie_ptr, s.get(), mask_opt);
  
  std::cout << "-> Doing init!" << std::endl;
  s->init();
  std::cout << "-> Doing preScan!" << std::endl;
  s->preScan();

  // Create folder
  //for some reason, 'make' issues that mkdir is an undefined reference
  //a test program on another machine has worked fine
  //a test program on this machine has also worked fine
  //    int mDExSt = mkdir(outputDir.c_str(), 0777); //mkdir exit status
  //    mode_t myMode = 0777;
  //    int mDExSt = mkdir(outputDir.c_str(), myMode); //mkdir exit status
  std::string cmdStr = "mkdir -p "; //I am not proud of this ):
  cmdStr += outputDir;
  int sysExSt = system(cmdStr.c_str());
  if(sysExSt != 0){
    std::cerr << "Error creating output directory - plots might not be saved!" << std::endl;
  }
  //read errno variable and catch some errors, if necessary
  //errno=1 is permission denied, errno = 17 is dir already exists, ...
  //see /usr/include/asm-generic/errno-base.h and [...]/errno.h for all codes

  // Run from downstream to upstream
  std::cout << "-> Starting histogrammer and analysis threads:" << std::endl;
  for ( FrontEnd* fe : bookie_ptr->feList ) {
    if (fe->isActive()) {
      analyses[fe]->init();
      analyses[fe]->run();
      
      histogrammers[fe]->init();
      histogrammers[fe]->run();
      
      std::cout << "  -> Analysis thread of Fe " << dynamic_cast<FrontEndCfg*>(fe)->getRxChannel() << std::endl;
    }
  }
  
  //Fei4DataProcessor proc(bookie_ptr->globalFe<Fei4>()->getValue(&Fei4::HitDiscCnfg));
  proc = StdDict::getDataProcessor(chipType);
  proc->connect( &bookie_ptr->rawData, &bookie_ptr->eventMap );
  proc->init();
  proc->run();

  if (scanType.find("noisescan") != std::string::npos){
    // Directly access loops for noise scan here
    (s->getLoop(0).get())->init();
    (s->getLoop(0).get())->execPart1();
    (s->getLoop(1).get())->init();
    (s->getLoop(1).get())->execPart1();

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(20)){
      (dynamic_cast<StdDataLoop*>(s->getLoop(1).get()))->execPart2Standalone();
    }

    std::cout << "Before casting in frontend stop" << std::endl;
    (dynamic_cast<StdDataLoop*>(s->getLoop(1).get()))->endScan();
    (dynamic_cast<StdDataLoop*>(s->getLoop(1).get()))->end();
    (dynamic_cast<Rd53aTriggerLoop*>(s->getLoop(0).get()))->execPart2();
    (dynamic_cast<Rd53aTriggerLoop*>(s->getLoop(0).get()))->end();
    std::cout << "after casting in frontend stop" << std::endl;
  }
  else s->run();

  s->postScan();
  std::cout << "-> Scan done!" << std::endl;

  // Join from upstream to downstream.
  
  proc->scanDone = true;
  bookie_ptr->rawData.cv.notify_all();

  std::cout << "-> Waiting for processors to finish ..." << std::endl;
  // Join Fei4DataProcessor
  proc->join();
      
  std::cout << "-> Processor done, waiting for histogrammer ..." << std::endl;
    
  Fei4Histogrammer::processorDone = true;
    
  for (unsigned i=0; i<bookie_ptr->feList.size(); i++) {
    FrontEnd *fe = bookie_ptr->feList[i];
    if (fe->isActive()) {
      fe->clipData->cv.notify_all();
    }
  }
    
  // Join histogrammers
  for( auto& histogrammer : histogrammers ) {
    histogrammer.second->join();
  }
    
  std::cout << "-> Processor done, waiting for analysis ..." << std::endl;
    
  Fei4Analysis::histogrammerDone = true;
    
  for (unsigned i=0; i<bookie_ptr->feList.size(); i++) {
    FrontEnd *fe = bookie_ptr->feList[i];
    if (fe->isActive()) {
      fe->clipHisto->cv.notify_all();
    }
  }
  
  // Join analyses
  for( auto& ana : analyses ) {
    ana.second->join();
  }
      
  std::cout << "-> All done!" << std::endl;

  // Joining is done.
  
  //hwCtrl->setCmdEnable(0x0);
  hwCtrl->setRxEnable(0x0);

  std::cout << std::endl;
  std::cout << "\033[1;31m###########\033[0m" << std::endl;
  std::cout << "\033[1;31m# Cleanup #\033[0m" << std::endl;
  std::cout << "\033[1;31m###########\033[0m" << std::endl;

  // Call constructor (eg shutdown Emu threads)
  hwCtrl.reset();

  // Need this folder to plot
  if (system("mkdir -p /tmp/$USER") < 0) {
    std::cerr << "#ERROR# Problem creating /tmp/$USER folder. Plots might work." << std::endl;
  }

  // Cleanup
  //delete s;
  for (unsigned i=0; i<bookie_ptr->feList.size(); i++) {
    FrontEnd *fe = bookie_ptr->feList[i];
    if (fe->isActive()) {
            
      // Save config
      std::cout << "-> Saving config of FE " << dynamic_cast<FrontEndCfg*>(fe)->getName() << " to " << feCfgMap.at(fe) << std::endl;
      json jTmp;
      dynamic_cast<FrontEndCfg*>(fe)->toFileJson(jTmp);
      std::ofstream oFTmp(feCfgMap.at(fe));
      oFTmp << std::setw(4) << jTmp;
      oFTmp.close();

      // Save extra config in data folder
      std::ofstream backupCfgFile(outputDir + dynamic_cast<FrontEndCfg*>(bookie_ptr->getLastFe())->getConfigFile() + ".after");
      json backupCfg;
      dynamic_cast<FrontEndCfg*>(bookie_ptr->getLastFe())->toFileJson(backupCfg);
      backupCfgFile << std::setw(4) << backupCfg;
      backupCfgFile.close(); 
      
      // Plot
      if (doPlots) {
	std::cout << "-> Plotting histograms of FE " << dynamic_cast<FrontEndCfg*>(fe)->getRxChannel() << std::endl;
	std::string outputDirTmp = outputDir;
	auto& ana = static_cast<Fei4Analysis&>( *(analyses[fe]) );
	ana.plot(dynamic_cast<FrontEndCfg*>(fe)->getName(), outputDirTmp);
	ana.toFile(dynamic_cast<FrontEndCfg*>(fe)->getName(), outputDir);
      }
    }
  }
  std::string lsCmd = "ls -1 " + outputDir + "*.p*";
  if (system(lsCmd.c_str()) < 0) {
    std::cout << "Find plots in: " << outputDir << std::endl;
  }
  return 0;
}

std::unique_ptr<ScanBase> buildScan( const std::string& scanType, Bookkeeper& bookie ) {
  std::unique_ptr<ScanBase> s ( nullptr );
 
    if (scanType.find("json") != std::string::npos) {
        std::cout << "-> Found Scan config, constructing scan ..." << std::endl;
        s.reset( new ScanFactory(&bookie) );
        std::ifstream scanCfgFile(scanType);
        if (!scanCfgFile) {
            std::cerr << "#ERROR# Could not open scan config: " << scanType << std::endl;
            throw("buildScan failure!");
        }
        json scanCfg;
        try {
            scanCfg = json::parse(scanCfgFile);
        } catch (json::parse_error &e) {
            std::cerr << "#ERROR# Could not parse config: " << e.what() << std::endl;
        }
        dynamic_cast<ScanFactory&>(*s).loadConfig(scanCfg);
    } else {
        std::cout << "-> Selecting Scan: " << scanType << std::endl;
        auto scan = StdDict::getScan(scanType, &bookie);
        if (scan != nullptr) {
            std::cout << "-> Found Scan for " << scanType << std::endl;
	            s = std::move(scan);
        } else {
            std::cout << "-> No matching Scan found, possible:" << std::endl;
            //listScans();
            std::cerr << "-> Aborting!" << std::endl;
            throw("buildScan failure!");
        }
    }
    return s;
}

void buildHistogrammers( std::map<FrontEnd*, std::unique_ptr<DataProcessor>>& histogrammers, const std::string& scanType, std::vector<FrontEnd*>& feList, ScanBase* s, std::string outputDir) {
    if (scanType.find("json") != std::string::npos) {
        std::cout << "-> Found Scan config, loading histogrammer ..." << std::endl;
        std::ifstream scanCfgFile(scanType);
        if (!scanCfgFile) {
            std::cerr << "#ERROR# Could not open scan config: " << scanType << std::endl;
            throw("buildHistogrammers failure!");
        }
        json scanCfg;
        scanCfg= json::parse(scanCfgFile);
        json histoCfg = scanCfg["scan"]["histogrammer"];
        json anaCfg = scanCfg["scan"]["analysis"];

        for (FrontEnd *fe : feList ) {
            if (fe->isActive()) {
                // TODO this loads only FE-i4 specific stuff, bad
                // Load histogrammer
                histogrammers[fe].reset( new Fei4Histogrammer );
                auto& histogrammer = static_cast<Fei4Histogrammer&>( *(histogrammers[fe]) );
                
                histogrammer.connect(fe->clipData, fe->clipHisto);
                int nHistos = histoCfg["n_count"];
                std::cout << nHistos << std::endl;
                for (int j=0; j<nHistos; j++) {
                    std::string algo_name = histoCfg[std::to_string(j)]["algorithm"];
                    if (algo_name == "OccupancyMap") {
                        std::cout << "  ... adding " << algo_name << std::endl;
                        histogrammer.addHistogrammer(new OccupancyMap());
                    } else if (algo_name == "TotMap") {
                        std::cout << "  ... adding " << algo_name << std::endl;
                        histogrammer.addHistogrammer(new TotMap());
                    } else if (algo_name == "Tot2Map") {
                        std::cout << "  ... adding " << algo_name << std::endl;
                        histogrammer.addHistogrammer(new Tot2Map());
                    } else if (algo_name == "L1Dist") {
                        histogrammer.addHistogrammer(new L1Dist());
                        std::cout << "  ... adding " << algo_name << std::endl;
                    } else if (algo_name == "HitsPerEvent") {
                        histogrammer.addHistogrammer(new HitsPerEvent());
                        std::cout << "  ... adding " << algo_name << std::endl;
                    } else {
                        std::cerr << "#ERROR# Histogrammer \"" << algo_name << "\" unknown, skipping!" << std::endl;
                    }
                }
                histogrammer.setMapSize(fe->geo.nCol, fe->geo.nRow);
            }
        }
    } else {
        // Init histogrammer and analysis
      for (FrontEnd *fe : feList ) {
            if (fe->isActive()) {
                // Init histogrammer per FE
                histogrammers[fe].reset( new Fei4Histogrammer );
                auto& histogrammer = static_cast<Fei4Histogrammer&>( *(histogrammers[fe]) );
                
                histogrammer.connect(fe->clipData, fe->clipHisto);
                // Add generic histograms
                histogrammer.addHistogrammer(new OccupancyMap());
                histogrammer.addHistogrammer(new TotMap());
                histogrammer.addHistogrammer(new Tot2Map());
                histogrammer.addHistogrammer(new L1Dist());
                histogrammer.addHistogrammer(new HitsPerEvent());
                if (scanType == "selftrigger") {
                    // TODO set proper file name
                    histogrammer.addHistogrammer(new DataArchiver((outputDir + "data.raw")));
                }
                histogrammer.setMapSize(fe->geo.nCol, fe->geo.nRow);
            }
        }
    }
}


void buildAnalyses( std::map<FrontEnd*, std::unique_ptr<DataProcessor>>& analyses, const std::string& scanType, Bookkeeper& bookie, ScanBase* s, int mask_opt) {
    if (scanType.find("json") != std::string::npos) {
        std::cout << "-> Found Scan config, loading analysis ..." << std::endl;
        std::ifstream scanCfgFile(scanType);
        if (!scanCfgFile) {
            std::cerr << "#ERROR# Could not open scan config: " << scanType << std::endl;
            throw( "buildAnalyses failed" );
        }
        json scanCfg;
        scanCfg = json::parse(scanCfgFile);
        json histoCfg = scanCfg["scan"]["histogrammer"];
        json anaCfg = scanCfg["scan"]["analysis"];

        for (FrontEnd *fe : bookie.feList ) {
            if (fe->isActive()) {
                // TODO this loads only FE-i4 specific stuff, bad
                // TODO hardcoded
                analyses[fe].reset( new Fei4Analysis(&bookie, dynamic_cast<FrontEndCfg*>(fe)->getRxChannel()) );
                auto& ana = static_cast<Fei4Analysis&>( *(analyses[fe]) );
                ana.connect(s, fe->clipHisto, fe->clipResult);
                
                int nAnas = anaCfg["n_count"];
                std::cout << "Found " << nAnas << " Analysis!" << std::endl;
                for (int j=0; j<nAnas; j++) {
                    std::string algo_name = anaCfg[std::to_string(j)]["algorithm"];
                    if (algo_name == "OccupancyAnalysis") {
                        std::cout << "  ... adding " << algo_name << std::endl;
                        ana.addAlgorithm(new OccupancyAnalysis());
                     } else if (algo_name == "L1Analysis") {
                        std::cout << "  ... adding " << algo_name << std::endl;
                        ana.addAlgorithm(new L1Analysis());
                     } else if (algo_name == "TotAnalysis") {
                        std::cout << "  ... adding " << algo_name << std::endl;
                        ana.addAlgorithm(new TotAnalysis());
                     } else if (algo_name == "NoiseAnalysis") {
                        std::cout << "  ... adding " << algo_name << std::endl;
                        ana.addAlgorithm(new NoiseAnalysis());
                     } else if (algo_name == "ScurveFitter") {
                        std::cout << "  ... adding " << algo_name << std::endl;
                        ana.addAlgorithm(new ScurveFitter());
                     } else if (algo_name == "OccGlobalThresholdTune") {
                        std::cout << "  ... adding " << algo_name << std::endl;
                        ana.addAlgorithm(new OccGlobalThresholdTune());
                     } else if (algo_name == "OccPixelThresholdTune") {
                        std::cout << "  ... adding " << algo_name << std::endl;
                        ana.addAlgorithm(new OccPixelThresholdTune());
                     }

                }
                // Disable masking of pixels
                if(mask_opt == 0) {
                    std::cout << " -> Disabling masking for this scan!" << std::endl;
                    ana.setMasking(false);
                }
                ana.setMapSize(fe->geo.nCol, fe->geo.nRow);
            }
        }
    } else {
        // Init histogrammer and analysis
      for (FrontEnd *fe : bookie.feList ) {
            if (fe->isActive()) {
                // Init analysis per FE and depending on scan type
                analyses[fe].reset( new Fei4Analysis(&bookie, dynamic_cast<FrontEndCfg*>(fe)->getRxChannel()) );
                auto& ana = static_cast<Fei4Analysis&>( *(analyses[fe]) );
                ana.connect(s, fe->clipHisto, fe->clipResult);
                ana.addAlgorithm(new L1Analysis());
                if (scanType == "digitalscan") {
                    ana.addAlgorithm(new OccupancyAnalysis());
                } else if (scanType == "analogscan") {
                    ana.addAlgorithm(new OccupancyAnalysis());
                } else if (scanType == "thresholdscan") {
                    ana.addAlgorithm(new ScurveFitter());
                } else if (scanType == "totscan") {
                    ana.addAlgorithm(new TotAnalysis());
                } else if (scanType == "tune_globalthreshold") {
                    ana.addAlgorithm(new OccGlobalThresholdTune());
                } else if (scanType == "tune_pixelthreshold") {
                    ana.addAlgorithm(new OccPixelThresholdTune());
                } else if (scanType == "tune_globalpreamp") {
                    ana.addAlgorithm(new TotAnalysis());
                } else if (scanType == "tune_pixelpreamp") {
                    ana.addAlgorithm(new TotAnalysis());
                } else if (scanType == "noisescan") {
                    ana.addAlgorithm(new NoiseAnalysis());
                } else if (scanType == "selftrigger") {
                    ana.addAlgorithm(new OccupancyAnalysis());
                    ana.getLastAna()->disMasking();
                } else if (scanType == "selftrigger_noise") {
                    ana.addAlgorithm(new NoiseAnalysis());
                } else {
                    std::cout << "-> Analyses not defined for scan type" << std::endl;
                    //listScans();
                    std::cerr << "-> Aborting!" << std::endl;
                    throw("buildAnalyses failure!");
                }
                // Disable masking of pixels
                if(mask_opt == 0) {
                    std::cout << " -> Disabling masking for this scan!" << std::endl;
                    ana.setMasking(false);
                }
                ana.setMapSize(fe->geo.nCol, fe->geo.nRow);
            }
        }
    }
}

