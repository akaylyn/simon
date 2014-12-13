// Button subunit.  Responsible for hard buttons on a PCB. 

#ifndef Button_h
#define Button_h

#include "Pinouts.h"
#include <Arduino.h>
#include <Simon_Comms.h> // sizes, indexing defines.

#include <Streaming.h> // <<-style printing

//----- manual buttons: hard buttons on a PCB. 
#include <Bounce.h> 
// debounce the hardware buttons on the console
#define BUTTON_DEBOUNCE_TIME 10UL // ms

// note that the buttons are wired to ground with a pullup resistor.
#define PRESSED_BUTTON LOW 

// configures buttons at startup
void buttonStart();

// check for a specific button change
boolean buttonChanged(byte lightIndex);

// returns true if any of the buttons have switched states.
boolean buttonAnyChanged();

// returns true if any of the buttons are pressed.
boolean buttonAnyPressed();

// returns true if a specific button has changed
boolean buttonChanged(byte buttonIndex);

// returns true if a specific button is pressed
boolean buttonPressed(byte buttonIndex);

#endif
