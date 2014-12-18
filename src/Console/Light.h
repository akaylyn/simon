// Light subunit.  Responsible for UX (light) output local to the console.  Coordinates outboard Light.

#ifndef Light_h
#define Light_h

// Light module IO.  Pull LOW to active touch buttons and rim lighting.
#define PIXELS_RED 31
#define PIXELS_YEL 33
#define PIXELS_GRN 35 
#define PIXELS_BLU 37

// Manual button lights, panels, under console.  wire to N-channel MOSFET + and any GND.
#define LED_YEL 8 // can move, PWM
#define LED_GRN 9 // can move, PWM
#define LED_BLU 10 // can move, PWM
#define LED_RED 11 // can move, PWM

#include <Arduino.h>
#include <Simon_Indexes.h> // sizes, indexing defines.

#include <LED.h> // with LED abstraction

#include <Streaming.h> // <<-style printing

// pixel on and off (digitalWrite).  use for pixelValue.
#define PIXELS_ON LOW 
#define PIXELS_OFF HIGH

// LED mins and max (analogWrite).  use for ledValue.
#define LED_ON 255 // analogWrite
#define LED_OFF 0  // analogWrite

// combined LED and pixel on/off.  use for ledValue.
#define LIGHT_ON 255
#define LIGHT_OFF 0

// it's possible we'd want to set LEDs to some intermediate brightness, but the Pixels are a binary affair.
// use this cutoff for mapping [0,255] (ledValue) to [0,1] (pixelValue).
// ledValue >= this value maps to pixelValue=1; pixelValue=0 otherwise.
#define PIXEL_THRESHOLD 128

// configures lights at startup
void lightStart();
// configures lights for buttons at startup
void ledStart();  
// configures lights on rim and buttons at startup
void pixelsStart();

// set pixel and LED (or I_ALL)  to a value
void lightSet(byte lightIndex, byte ledValue);
// set led light (or I_ALL) to a value
void ledSet(byte lightIndex, byte ledValue);
// set pixel light (or I_ALL) to a value
void pixelSet(byte lightIndex, byte pixelValue);

#endif
