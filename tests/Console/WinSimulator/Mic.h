#ifndef Mic_h
#define Mic_h

#include <Arduino.h>
#include <Streaming.h> // <<-style printing

// MSGEQ7 datasheet: https://www.sparkfun.com/datasheets/Components/General/MSGEQ7.pdf
// pin locations

#define MSGEQ7_ANALOG_PIN      A0
#define MSGEQ7_STROBE_PIN      A1
#define MSGEQ7_RESET_PIN       A2

#define NUM_FREQUENCY_BANDS    7

#define NUM_SAMPLES 20

#define DEFAULT_THRESHOLD 4.0 // just a suggesting.  setThreshold can can tune this.  

// see this discussion on what instruments appear in what band:
// http://homerecording.com/bbs/general-discussions/mixing-techniques/frequency-charts-50110/

class Mic {
  public:
    // startup
    void begin();
    
    // show the volume levels 
    void print();
    
    // read the current volume levels, computes some additional information
    void update();
     
    // convenience extraction functions
    int getVol(byte band);
    bool getBeat(byte band); // flag a beat in the band if currVol >= volAvg + threshold*volSD.
    
    float getAvg(byte band);
    float getSD(byte band);
    float getTh(byte band);
    void setThreshold(byte band, float threshold); 
      
  private:
    // index for last read
    int sampleIndex;
    // track the volume
    float bandVol[NUM_FREQUENCY_BANDS][NUM_SAMPLES];
     // band volume running average
    float bandAvg[NUM_FREQUENCY_BANDS];
    // band volume standard deviation
    float bandSD[NUM_FREQUENCY_BANDS];
    // threshold settings, by band, for beat detection
    float bandTh[NUM_FREQUENCY_BANDS];
    // beat tracking
    bool isBeat[NUM_FREQUENCY_BANDS];
};
    
extern Mic mic;

#endif
