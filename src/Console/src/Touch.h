// capsense touch: MPR121 include I2C and MPR121 libraries.

#ifndef Touch_h
#define Touch_h

#include "Pinouts.h"
#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

#include <Wire.h>
#ifndef MPR121_h 
#define MPR121_h
#include <Adafruit_MPR121.h>
#endif

// 
void touchStart(uint8_t touchCount, uint8_t releaseCount);
void touchStart();

// unit test for touch sensor
void touchUnitTest(boolean details, unsigned long timeout=10000);

#endif
