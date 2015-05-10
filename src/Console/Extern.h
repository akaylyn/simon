// Extern subunit.  Responsible for interfacing with other projects via RFM12b.

#ifndef Extern_h
#define Extern_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers.

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h> 

#include <RFM12B.h> // Console
#include "Light.h" // for executing instructions and network maintenance

// once we get radio comms, wait this long  before returning false from externUpdate.
#define EXTERNAL_COMMS_TIMEOUT 10000UL

// sets up external interface
void externStart();

// checks for external traffic; if traffic, responds to it and returns true
boolean externUpdate();

#endif
