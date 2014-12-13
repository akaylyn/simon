// Extern subunit.  Responsible for interfacing with other projects via RFM12b.

#ifndef Extern_h
#define Extern_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing

// sets up external interface
void externStart();

// checks for external traffic; if traffic, responds to it and returns true
boolean externUpdate();

#endif
