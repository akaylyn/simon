// Lights subunit.  Controls IR blaster and accumulator lighting

#ifndef Light_h
#define Light_h

#include <Arduino.h>

#include <RGBlink.h> // control LEDs
#include <IRremote.h> // control IR Rx lighting
#include <QueueArray.h> // queing for IR transmissions
#include <Metro.h> // countdown timers

#include <Streaming.h> // <<-style printing

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

class Flood {
  public:
    // startup
    void begin(byte floodPin, unsigned long sendInterval = 2UL, byte sendCount = 3);

    // update routine for resends
    void update();
    // set color
    void setColor(colorInstruction &color);

  protected:
    void on();
    void off();

    void setBright(byte level);
    void queCode(unsigned long data);
    void dropQue();
    void sendCode(unsigned long data);

    colorInstruction currentColor;
    int currentBright;
    boolean isOn;

    // que control for IR blaster
    QueueArray <unsigned long> que;
    unsigned long sendInterval; // Need to wait before resending IR packets.
    byte sendCount; // NEC IR protocol requires packets get sent in triplicate. Testing suggests duplicate is enough.

    // Use NEC IR protocol
    IRsend *ir;
    
    byte avgIntensity(unsigned long c1, unsigned long c2, unsigned long c3);
    byte avgIntensity(unsigned long c1, unsigned long c2);
    byte intensityToBright(byte intensity);

};


// different lighting modes available.
enum lightEffect_t {
  Solid = 0, // always on
  Blink = 1, // blinking with intervals
  Fade = 2 // soft fading
};

class Light {
  public:

  void begin(byte redPin, byte greenPin, byte bluePin, byte floodPin);
  void update();
  void perform(colorInstruction &inst);
  void effect(lightEffect_t effect = Solid, uint16_t onTime = 1000UL, uint16_t offTime = 100UL);

  private:
  
  // RGB lighting tied together on tank
  LED *tank;
  // RGB lighting flood
  Flood *flood;
};

// NEC IR protocol commands
// 8 bits address and complement; 8 bits command and complement
//                     command   comp c
#define K24_OFF          0xF740BF // 0000 0000 1111 0111 0100 0000 1011 1111
#define K24_ON           0xF7C03F // 0000 0000 1111 0111 1100 0000 0011 1111

#define K24_RED          0xF720DF // 0000 0000 1111 0111 0010 0000 1101 1111
#define K24_GRN          0xF7A05F // 0000 0000 1111 0111 1010 0000 0101 1111
#define K24_BLU          0xF7609F // 0000 0000 1111 0111 0110 0000 1001 1111
#define K24_YEL          0xF728D7 // 0000 0000 1111 0111 0010 1000 1101 0111
#define K24_WHT          0xF7E01F // 0000 0000 1111 0111 1110 0000 0001 1111

#define K24_DOWN         0xF7807F // 0000 0000 1111 0111 1000 0000 0111 1111
#define K24_UP           0xF700FF // 0000 0000 1111 0111 0000 0000 1111 1111

#define K24_FLASH        0xF7D02F // 0000 0000 1111 0111 1101 0000 0010 1111
#define K24_STROBE       0xF7F00F // 0000 0000 1111 0111 1111 0000 0000 1111
#define K24_FADE         0xF7C837 // 0000 0000 1111 0111 1100 1000 0011 0111
#define K24_SMOOTH       0xF7E817 // 0000 0000 1111 0111 1110 1000 0001 0111

#define K24_REPEAT       0xFFFFFF // 0000 0000 1111 1111 1111 1111 1111 1111

#define N_BRIGHT_STEPS   5

// not used, but here's the full 24-key pallete
#define COL_WHITE_C   0xF7E01F // 1110 0000
#define COL_VLBLUE_C  0xF7B04F // 1011 0000
#define COL_MBLUE_C   0xF7A857 // 1010 1000
#define COL_GREEN_C   0xF7A05F // 1010 0000
#define COL_LGREEN_C  0xF7906F // 1001 0000
#define COL_MLBLUE_C  0xF78877 // 1000 1000

#define COL_PURPLE_C  0xF7708F // 0111 0000
#define COL_BLUE_C    0xF7609F // 0110 0000
#define COL_LBLUE_C   0xF750AF // 0101 0000
#define COL_LPINK_C   0xF748B7 // 0100 1000
#define COL_PINK_C    0xF76897 // 0110 1000

#define COL_MORANGE_C 0xF730CF // 0011 0000
#define COL_RED_C     0xF720DF // 0010 0000
#define COL_YELLOW_C  0xF728D7 // 0010 1000

#define COL_ORANGE_C  0xF710EF // 0001 0000
#define COL_LORANGE_C 0xF708F7 // 0000 1000


#endif

