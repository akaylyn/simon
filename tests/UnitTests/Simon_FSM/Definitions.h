#ifndef DEFS_H
#define DEFS_H

// serial rates
#define DEBUG_RATE 115200

// what do we need to run to the sculpture?
/*
1 - GND - also tie to 110VAC GND on both sides for high amp panel frames
2 - RED - panel lights, +12
3 - GRN - panel lights, +12
4 - YEL - panel lights, +12
5 - BLU - panel lights, +12
6 - SPARKY - arc ignitor, +5 to trigger SSR
7 - ACCUM - accumulator release, +5 to trigger SSR
8 - PRESS - pressure sensor, input
*/ 

// *** Arm System
// wire a switch and a key inline with the ACCUM line.  run a 10K resistor to GND to pull down the ACCUM line.
// this generates a hardware-level ARM-ENABLE system, which the software cannot override

// *** System Inputs ***
// ****** Switches
// button switches.  wire to Arduino (INPUT_PULLUP) and GND.
#define RED_SWITCH 24
#define GRN_SWITCH 26
#define BLU_SWITCH 28
#define YEL_SWITCH 30
// debounce the buttons on the console
#define DEBOUNCE_TIME 10UL // ms
// note that the buttons are wired to ground with a pullup resistor.
#define PRESSED LOW 

// ****** Pressure Sensor
// accumulator pressure
#define PRESSURE_WIRE A0

// pressure sensor readings
// sensor generates 0.5V at 0 psig and 5V at 100 psig
#define PRESSURE_MIN 96 // =1023/(0.5/5)
#define PRESSURE_MAX 1023

// ****** IR Sensor
// Wiring directives:
// place PIR to get unobstructed view
// leftmost pin on PIR to this pin.  
#define PIR_WIRE 32
// middle PIR wire to ground
// rightmost PIR wire to +12V
#define PIR_TRIGGERED LOW

// define the minimum time between motion events if fanfare isn't ready
#define MOTION_INTERVAL 3000UL // ms

// ****** Analog Knobs
// new pots attached to A4 and A5
#define POT1_WIRE A4
#define POT2_WIRE A5
// reading range
#define POT_MIN 65
#define POT_MAX 230

// debounce the analog readings
#define ANALOG_DEBOUNCE_COUNT 20

// *** System Outputs ***
// ****** Lighting
// button lights, panels, under console.  wire to N-channel MOSFET + and any GND.
#define RED_LIGHT 3
#define GRN_LIGHT 4
#define BLU_LIGHT 5
#define YEL_LIGHT 6
// LED mins and max
#define LED_OFF 0  // analogWrite
#define LED_ON 255 // analogWrite
// probability (fraction) that a panel turns on for each note
#define PANEL_ON_FRACTION 3

// ****** Sound
// speaker
#define SPEAKER_WIRE 7 // PWM

// tone frequencies for each note
#define RED_TONE NOTE_DS4    // Red ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Å“ 310 Hz ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Å“ D#4 (true pitch 311.127 Hz)
#define GRN_TONE NOTE_GS4    // Green ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Å“ 415 Hz ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Å“ G#4 (true pitch 415.305 Hz)
#define BLU_TONE NOTE_GS3    // Blue ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Å“ 209 Hz ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Å“ G#3 (true pitch 207.652 Hz) 
#define YEL_TONE NOTE_B3    // Yellow ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬Å“ 252 Hz ÃƒÂ¢Ã¢â€šÂ¬Ã¯Â¿Â½ B3 (true pitch 247.942 Hz)
#define WRONG_TONE 42   // a losing tone of 42 Hz 


// ****** Ignitor

// arc ignitor
#define SPARKY_WIRE 8

// define the gap ignitor relight interval
#define SPARKY_RELIGHT_INTERVAL 300000UL // 5 min
// define gap igniter on time
#define SPARKY_DURATION 5000UL

// SSR and MOSFETs have different TTL levels for on and off
#define SPARKY_ON HIGH
#define SPARKY_OFF LOW

// ****** Accumulator

// accumulator solenoid
#define ACCUM_WIRE 9

// SSR and MOSFETs have different TTL levels for on and off
#define ACCUM_ON HIGH
#define ACCUM_OFF LOW

// define the timing of the poofs
#define POOF_MIN 25UL
#define POOF_MODE 100UL
#define POOF_MAX 300UL
// pause between poofs
#define WAIT_MIN 150UL
#define WAIT_MAX 500UL
// define the number of poofs to throw 
#define MIN_POOF_COUNT 2
#define MAX_POOF_COUNT 4

// define the minimum interval between poofs
#define MINIMUM_FIRING_INTERVAL 1000UL // 1 second

// leave the flame on for 1/FLAME_ON_FRACTION of the note duration
//#define FLAME_ON_FRACTION 3UL
// probability (percentage) that the flame pops on each fanfare note
#define FLAME_ON_PERCENT 42

// *** Playing Model

// get this number in a row, and you're super cheatypants.
#define MAX_SEQUENCE_LENGTH 25 // good luck with that.
// get this number or higher for a real song
#define LEVEL_1 6
// get this number of higher for awesome songs
#define LEVEL_2 12

// when playing, a player can wait this interval between button presses before losing.
#define PLAYER_TIMEOUT 2000UL // 2 seconds

// enable this to press anything to match the last sequence
#define CHEATY_PANTS_MODE false

// *** Kiosk Mode

// define the minimum time between fanfares in kiosk mode  <- influenced by knob 1
#define KIOSK_FANFARE_MIN 30000UL  // 30 seconds
#define KIOSK_FANFARE_MAX 300000UL  // 5 minutes

#endif



