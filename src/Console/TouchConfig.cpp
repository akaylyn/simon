#include "TouchConfig.h"
#include <MPR121.h> // config struct
#include <Streaming.h> 

static const byte EEPROM_LOC = 110; // size of storage is sizeof MPR121_settings_t?

void TouchConfig::setup() {
    MPR121_settings_t config1 {};
// Since a touch can only be in the decreasing direction, it is usually best to
// set the decreasing filter to be slower that the increasing one. This allows for automatic recovery from a bad baseline reading.
    /*
        use a balloon to build up a static charge
        can we build the conditions like at the burn to test?
        mike: can we run a cycle will we have someone touch and release each button
        in order to set the bounds based on the environment
    */
    // set config 1
    config1.TTHRESH = 70;
    config1.RTHRESH = 40;

    /* MHDR
    Max Half Delta – The Max Half Delta determines the largest magnitude of
    variation to pass through the third level filter. 1-63
    */
    config1.MHDR = 0x2D;
    config1.MHDF = 0x0F;

    /*
    
    Noise Half Delta – The Noise Half Delta determines the incremental change when
    non-noise drift is detected.

    1-63
    */
    config1.NHDR = 0x3F;
    config1.NHDF = 0x08;

    /*
    Noise Count Limit – The Noise Count Limit determines the number of samples consecutively greater
    than the Max Half Delta necessary before it can be determined that it is non-noise.
    00000000 Encoding 0 – Sets the Noise Count Limit to 1 (every time over Max Half Delta)
    00000001 Encoding 1 – Sets the Noise Count Limit to 2 consecutive samples over Max Half Delta
    ~
    11111111 Encoding 255 – Sets the Noise Count Limit to 255 consecutive samples over Max Half Delta
    */
    config1.NCLR = 0x0A;
    config1.NCLF = 0x01;


    /*
    Filter Delay Limit – The Filter Delay Limit determines the rate of operation of the
    filter. A larger number makes it operate slower.
    00000000 Encoding 0 – Sets the Filter Delay Limit to 1
    00000001 Encoding 1 – Sets the Filter Delay Limit to 2
    ~
    11111111 Encoding 255 – Sets the Filter Delay Limit to 255
    */
    config1.FDLR = 0x03;
    config1.FDLF = 0x0C;

    config1.NHDT = 0x04;
    config1.NCLT = 0x01;


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