#include "TouchConfig.h"
#include <MPR121.h> // config struct
#include <Streaming.h> 

void TouchConfig::setup() {
    MPR121_settings_t config1 {};

    // set config 1
    config1.TTHRESH = 70;
    config1.RTHRESH = 40;

    config1.MHDR = 0x2D;
    config1.NHDR = 0x3F;
    config1.NCLR = 0x01;
    config1.FDLR = 0x03;

    config1.NHDT = 0x04;
    config1.NCLT = 0x01;

    config1.MHDF = 0x0F;
    config1.NHDF = 0x08;
    config1.NCLF = 0x0A;
    config1.FDLF = 0x0C;
    // set config 2
    // set config 3
}

// configset 1, 2, 3
void TouchConfig::setConfig(int configSet) {

    if (configSet == 1){
        Serial << "Config Set 1" << endl;
    } else if (configSet == 2){
        Serial << "Config Set 2" << endl;

    } else if (configSet == 3){
        Serial << "Config Set 3" << endl;

    }

}