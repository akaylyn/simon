// Light subunit.  Responsible for UX (light) output local to the console.  Coordinates outboard Light.

#ifndef Light_h
#define Light_h

#include <Arduino.h>
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

// for fireEnable function
#include "Sensor.h" 
extern Sensor sensor;

// LED abstracting
#include <LED.h> 

// radio
#include <SPI.h> // radio transmitter is a SPI device
#include <EEPROM.h> // saving and loading radio settings
#include <RFM12B.h> // RFM12b radio transmitter module

// Light module IO.  Pull LOW to active touch buttons and rim lighting.
#define PIXELS_RED 36
#define PIXELS_BLU 38
#define PIXELS_GRN 40
#define PIXELS_YEL 42

// note wired up to enable SoftwareSerial comms btw Light and Mega/Console.  Not currently used, but go for it.
#define SS_TX 34
#define SS_RX 44

// Manual button lights, panels, under console.  wire to N-channel MOSFET + and any GND.
#define LED_YEL 8 // can move, PWM
#define LED_GRN 9 // can move, PWM
#define LED_BLU 10 // can move, PWM
#define LED_RED 11 // can move, PWM


// SPI library requirements: http://arduino.cc/en/Reference/SPI
#define RADIO_SCK 52 // SPI CLK/SCK
#define RADIO_SDO 50 // SPI MISO/SDI
#define RADIO_SDI 51 // SPI MOSI/SDO
#define RADIO_SEL 53 // SPI SS/SEL
#define RADIO_IRQ 2 // IRQ 0
#define D_CS_PIN 10 // default SS pin for RFM module


#define CONFIG_SEND_INTERVAL 30000UL // ms. 
#define SEND_INTERVAL 5UL
#define PING_COUNT 10



// LED mins and max (analogWrite).  use for ledValue.
#define LIGHT_ON 255
#define LIGHT_OFF 0

// pixel on and off (digitalWrite).  use for pixelValue.
#define PIXEL_ON LOW
#define PIXEL_OFF HIGH

// it's possible we'd want to set LEDs to some intermediate brightness, but the Pixels are a binary affair.
// use this cutoff for mapping [0,255] (ledValue) to [0,1] (pixelValue).
// ledValue >= this value maps to pixelValue=1; pixelValue=0 otherwise.
#define PIXEL_THRESHOLD LIGHT_ON>>2

// if we ask for resends, how many?
#define RESEND_COUNT 3

class Light {
  public:
    // startup
    void begin();

    // set light level
    void setLight(byte index, byte level, boolean showNow = false, byte nodeID = 0);
    void setAllLight(byte level, boolean showNow = false, byte nodeID = 0);

    // set fire level
    void setFire(byte index, byte level, boolean showNow = false, byte nodeID = 0);
    void setAllFire(byte level, boolean showNow = false, byte nodeID = 0);

    // set light and fire off
    void setAllOff(boolean showNow = true, byte nodeID = 0);

    // call this to showshowNow the light and fire settings
    void show(byte nodeID = 0);

    // call this to perform resend and network maintenace
    void update();
    
    // take a towerInstruction and execute
    void sendInstruction(towerInstruction &externInst);

  private:
    // storage for light and fire levels.
    towerInstruction inst;

    // storage for tower configurations
    towerConfiguration config[N_TOWERS];

    // track remaining resends
    unsigned int resendCount;

    // hardware LED
    LED *led[N_COLORS];  // messy, but I can't figure out how to declare without instantiation, which the compiler requires.

    // instantiates radio communications
    byte networkStart();
    
    // ping network for quality
    void networkPing(int count=PING_COUNT);

    // configure network
    void networkConfig();
    void towerConfig(towerConfiguration & config, byte nodeID);
};

extern Light light;

#endif
