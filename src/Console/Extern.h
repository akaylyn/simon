// Extern subunit.  Responsible for interfacing with other projects via RFM12b.

#ifndef Extern_h
#define Extern_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers.
#include <Simon_Comms.h> // towerInstruction definition.
#include <Simon_Indexes.h> // indexing.

// once we get radio comms, wait this long  before returning false from externUpdate.
#define EXTERNAL_COMMS_TIMEOUT 10000UL

// sets up external interface
void externStart();

// checks for external traffic; if traffic, responds to it and returns true
boolean externUpdate();

// takes a towerInstruction and plays that out on the Console.  meh.
void externDoInstruction();
// set pixel light (or I_ALL) to a value.  Defined in Light.cpp.
extern void pixelSet(byte lightIndex, byte pixelValue);

#endif
