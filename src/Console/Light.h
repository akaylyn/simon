// Light subunit.  Responsible for UX (light) output local to the console.  Coordinates outboard Light.

#ifndef Light_h
#define Light_h

#include <Arduino.h>
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

#include <EasyTransfer.h>

// for send function
#include "Network.h"
extern Network network;

// LED abstracting
#include <LED.h> 

// Manual button lights, panels, under console.  wire to N-channel MOSFET + and any GND.
#define LED_YEL 8 // can move, PWM
#define LED_GRN 9 // can move, PWM
#define LED_BLU 10 // can move, PWM
#define LED_RED 11 // can move, PWM

class Light {
  public:
    // startup.  layout the towers.
    void begin(nodeID layout[N_COLORS], Stream *lightModuleSerial);

    // set fire level, taking advantage of layout position
    void setLight(color position, byte red, byte green, byte blue);
    void setLight(color position, colorInstruction &inst);
    // set fire level, ignoring tower positions (good luck with that)
    void setLight(nodeID node,  byte red, byte green, byte blue);
    void setLight(nodeID node, colorInstruction &inst);

    void clear(); // clear and send
    
  private:
    // storage for light and fire levels.
    colorInstruction inst[N_COLORS];

    // stores which towers should be sent color commands
    nodeID layout[N_COLORS];
    
    // local lighting
    void showLocal(color position, colorInstruction &inst);
    
    // hardware LED
    LED *led[N_COLORS];  
 
    // Light module comms   
    lightModuleInstruction ETinst;
    EasyTransfer ET; 

};

extern Light light;

#endif
