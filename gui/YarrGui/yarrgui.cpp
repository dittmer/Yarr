#include "yarrgui.h"
#include "ui_yarrgui.h"

#include <BitFile.h>
#include <BenchmarkTools.h>

#include <iostream>
#include <fstream>
#include <string>

#include <QMessageBox>

YarrGui::YarrGui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::YarrGui)
{
    ui->setupUi(this);

    this->init();

    // Get list of devices
    devicePath.setPath("/dev/");
    QStringList filter;
    filter << "spec*";
    devicePath.setFilter(QDir::System);
    deviceList = devicePath.entryList(filter);
    
    // Display devices in list
    ui->device_comboBox->addItems(deviceList);

    // Init device vector
    for(int i=0; i<deviceList.size(); i++) {
        specVec.push_back(new SpecController());
    }

    ui->global_cfg_checkBox->setChecked(true); //DEBUG
    ui->configfileName_2->setText("util/global_config.gcfg"); //DEBUG

    // Init console
//    qout = new QDebugStream(std::cout, ui->console, QColor("black"));
//    qerr = new QDebugStream(std::cerr, ui->console, QColor("red"));

    ui->main_tabWidget->setCurrentIndex(0);
    ui->main_tabWidget->setTabEnabled(0, true);
    ui->main_tabWidget->setTabEnabled(1, false);
    ui->main_tabWidget->setTabEnabled(2, false);
    ui->addFuncButton->setEnabled(false);

    ui->feTree->setColumnWidth(0, 200);
    ui->feTree->setColumnWidth(1, 500);

    ui->scanVec_lineEdit->setReadOnly(true);

    ui->plotTree->setColumnWidth(0, 500);
    ui->runCustomScanButton->setEnabled(false);
    ui->scanProgressBar->setValue(0);

    ui->additionalFunctionality->addItem("");
    ui->additionalFunctionality->addItem("Benchmark");
    ui->additionalFunctionality->addItem("EEPROM");
    ui->additionalFunctionality->addItem("Create scan");
}

YarrGui::~YarrGui()
{
    // Clean up devices
    for(int i=0; i<deviceList.size(); i++) {
        if (specVec[i]->isInitialized())
            delete specVec[i];
    }

    delete ui;
}

int YarrGui::getDeviceComboBoxCurrentIndex() {
    return ui->device_comboBox->currentIndex();
}

int YarrGui::getDeviceListSize() {
    return deviceList.size();
}

bool YarrGui::isSpecInitialized(unsigned int i) {
    return specVec.at(i)->isInitialized();
}

SpecController * YarrGui::specVecAt(unsigned int i) {
    return specVec.at(i);
}

void YarrGui::init() {
    // Preset labels
    ui->specid_value->setNum(-1);
    ui->bar0_value->setNum(-1);
    ui->bar4_value->setNum(-1);
}

void process(Bookkeeper *bookie, bool * scanDone) {
    // Set correct Hit Discriminator setting, for proper decoding
    Fei4DataProcessor proc(bookie->getGlobalFe()->getValue(&Fei4::HitDiscCnfg));
    proc.connect(&bookie->rawData, &bookie->eventMap);
    proc.init();

    while(!(*scanDone)) {
        // TODO some better wakeup signal?
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        proc.process();
    }
    proc.process();
}

void analysis(Fei4Histogrammer *h, Fei4Analysis *a, bool * processorDone) {
    h->init();
    a->init();

    while(!(*processorDone)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        h->process();
        a->process();
    }
    h->process();
    a->process();

    a->end();
}

//Simulates digital scan data
//Used for plot testing
//No longer needed
//void YarrGui::on_debugScanButton_clicked() {
//    QString qn = "Debug";
//    for(int j = 0; j < ui->feTree->topLevelItemCount(); j++) {
//        scanDone = false;
//        processorDone = false;

//        if(!(ui->feTree->topLevelItem(j)->child(4)->checkState(1))) { continue; } //Is the Scan checkbox checked?

//        QTreeWidgetItem * plotTreeItem = nullptr;
//        for(int k = 0; k < ui->plotTree->topLevelItemCount(); k++) { //Is the current FE already in the tree? ...
//           if(ui->plotTree->topLevelItem(k)->text(0) == ui->feTree->topLevelItem(j)->text(0)) {
//               plotTreeItem = ui->plotTree->topLevelItem(k);
//               break;
//           }
//        }

//        if(plotTreeItem == nullptr) { //... if not: create a branch fot it
//            plotTreeItem = new QTreeWidgetItem(ui->plotTree);
//            plotTreeItem->setText(0, ui->feTree->topLevelItem(j)->text(0));
//        }

//        QTreeWidgetItem * plotTreeItemDS = new QTreeWidgetItem();
//        plotTreeItemDS->setText(0, qn);
//        plotTreeItem->addChild(plotTreeItemDS);

//        QCustomPlot * tabScanPlot = new QCustomPlot();
//        QString newTabName = qn + ' ' + ui->feTree->topLevelItem(j)->text(0);
//        ui->scanPlots_tabWidget->addTab(tabScanPlot, newTabName);

//        QTreeWidgetItem * plotTreeItemP = new QTreeWidgetItem();
//        plotTreeItemP->setText(0, "Plot " + QString::number(plotTreeItemDS->childCount() + 1));
//        plotTreeItemDS->addChild(plotTreeItemP);

//        QCPColorMap * colorMap = new QCPColorMap(tabScanPlot->xAxis, tabScanPlot->yAxis);
//        tabScanPlot->addPlottable(colorMap);
//        colorMap->data()->setSize(80, 336);
//        colorMap->data()->setRange(QCPRange(0, 80), QCPRange(0, 336));
//        for(int xCoord = 0; xCoord<80; xCoord++) {
//            for(int yCoord = 0; yCoord<336; yCoord++) {
//                colorMap->data()->setCell(xCoord, yCoord, 25);
//            }
//        }

//        QCPColorScale * colorScale = new QCPColorScale(tabScanPlot);
//        tabScanPlot->plotLayout()->addElement(0, 1, colorScale);
//        colorScale->setType(QCPAxis::atRight);
//        colorMap->setColorScale(colorScale);
//        colorScale->axis()->setLabel(" ");
//        colorMap->setGradient(QCPColorGradient::gpPolar);
//        colorMap->rescaleDataRange();

//        tabScanPlot->rescaleAxes();
//        tabScanPlot->replot();

//    }
//    return;
//}

void YarrGui::on_addFuncButton_clicked()
{
    if(ui->additionalFunctionality->currentText() == "Benchmark") {
        BenchmarkDialog * myDialog = new BenchmarkDialog(this);
        myDialog->setModal(false);
        myDialog->setWindowTitle("Benchmark");
        myDialog->show();
    }
    if(ui->additionalFunctionality->currentText() == "EEPROM") {
        EEPROMDialog * myDialog = new EEPROMDialog(this);
        myDialog->setModal(false);
        myDialog->setWindowTitle("SpecBoard EEPROM");
        myDialog->show();
    }
    if(ui->additionalFunctionality->currentText() == "Create scan") {
//        CreateScanDialog * myDialog = new CreateScanDialog(bk, this);
//        myDialog->setModal(false);
//        myDialog->setWindowTitle("Create scan");
//        myDialog->show();
        //Less heap memory consumption...
        CreateScanDialog myDialog(bk, this);
        myDialog.setModal(false);
        myDialog.setWindowTitle("Create scan");
        myDialog.show();
        while(myDialog.isVisible()) {
            QApplication::processEvents();
        }
    }

    return;
}

#include "yarrgui_spec.cpp"
#include "yarrgui_fes.cpp"
#include "yarrgui_scans.cpp"
#include "yarrgui_plots.cpp"
//It seems that qt must have all code that belongs
//to a *.ui file in the corresponding *.cpp file,
//else it won't compile???
