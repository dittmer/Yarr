#include "Fei4DataProcessor.h"

#include <iostream>

Fei4DataProcessor::Fei4DataProcessor(unsigned arg_hitDiscCfg) : DataProcessor(){
    input = NULL;
    output = NULL;
    hitDiscCfg = arg_hitDiscCfg;
    totCode = {{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14, 0},
                  {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 1, 0},
                  {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 1, 0}}};
    

}

Fei4DataProcessor::~Fei4DataProcessor() {

}

void Fei4DataProcessor::init() {

}

void Fei4DataProcessor::process() {
    unsigned l1id = 0;
    unsigned bcid = 0;
    while(!input->empty()) {
        // Get data containers
        RawData *curIn = input->popData();
        Fei4Data *curOut = new Fei4Data();

        // Process
        int hits = 0;
        int events = 0;
        for (unsigned i=0; i<curIn->words; i++) {
            uint32_t value = curIn->buf[i];
            if (((value & 0x00FF0000) >> 16) == 0xe9) {
                // Pixel Header
                l1id = (value & 0x7c00) >> 10;
                bcid = (value & 0x03FF);
                curOut->newEvent(l1id);
                events++;
            } else if (((value & 0x00FF0000) >> 16) == 0xef) {
                // Service Record
                unsigned code = (value & 0xFC00) >> 10;
                unsigned number = value & 0x03FF;
                curOut->serviceRecords[code]+=number;
            } else if (((value & 0x00FF0000) >> 16) == 0xea) {
                // Address Record
            } else if (( value& 0x00FF0000) >> 16 == 0xec) {
                // Value Record
            } else {
                if (events == 0 ) {
                    curOut->newEvent(l1id);
                    events++;
                }
                unsigned col = (value & 0xFE0000) >> 17;
                unsigned row = (value & 0x01FF00) >> 8;
                unsigned tot1 = (value & 0xF0) >> 4;
                unsigned tot2 = (value & 0xF);
                if (i > 0 && (col == 0 || row == 0))
                    std::cout << "Someting wrong: " << i << " " << curIn->words << " " << std::hex << value << std::dec << std::endl;
                if (totCode[hitDiscCfg][tot1] > 0) {
                    if (curOut != NULL)
                        curOut->curEvent->addHit(bcid, row, col, totCode[hitDiscCfg][tot1]);
                    hits++;
                }
                if (totCode[hitDiscCfg][tot1] > 0) {
                    if (curOut != NULL)
                        curOut->curEvent->addHit(bcid, row+1, col, totCode[hitDiscCfg][tot2]);
                    hits++;
                }
            }
        }
        output->pushData(curOut);
        //Cleanup
        delete curIn;
    }
}
