// communication to Light module, controlling the rim and button lighting.

#ifndef Light_h
#define Light_h

#include "Pinouts.h"
#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

// configures Lights on rim and touch buttons at startup
void configureLights();

#endif
