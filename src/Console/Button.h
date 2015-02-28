// Button subunit.  Responsible for hard buttons on a PCB. 

#ifndef Button_h
#define Button_h

// Manual Buttons
// button switches.  wire to Arduino (INPUT_PULLUP) and GND.
#define BUTTON_YEL 4 // can move, digital
#define BUTTON_GRN 5 // can move, digital
#define BUTTON_BLU 6 // can move, digital
#define BUTTON_RED 7 // can move, digital

#include <Arduino.h>

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h> 

#include <Streaming.h> // <<-style printing

//----- manual buttons: hard buttons on a PCB. 
#include <Bounce.h> 
// debounce the hardware buttons on the console
#define BUTTON_DEBOUNCE_TIME 10UL // ms

// note that the buttons are wired to ground with a pullup resistor.
#define PRESSED_BUTTON LOW 

// configures buttons at startup
boolean buttonStart();

// returns true if any of the buttons have switched states.
boolean buttonAnyChanged();

// returns true if a specific button has changed
boolean buttonChanged(byte buttonIndex);

// returns true if any of the buttons are pressed.
boolean buttonAnyPressed();

// returns true if a specific button is pressed
boolean buttonPressed(byte buttonIndex);

// unit test for buttons
void buttonUnitTest();

#endif
