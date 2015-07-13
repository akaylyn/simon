// Compile for Mega

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// have: 2K memory, 3 bytes memory per LED, 60 LEDs per meter.
// so, we can support 2000/3/60=11.1 meters, other memory usage notwithstanding.

#include "Strip.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <Streaming.h>
#include <Metro.h>
#include <EasyTransfer.h>
#include <Simon_Common.h>
#include "ConcurrentAnimator.h"
#include "AnimationConfig.h"
#include "Animations.h"
#include "ConcurrentAnimator.h"
#include "AnimateFunc.h"

extern Adafruit_NeoPixel rimJob;
extern Adafruit_NeoPixel redL;
extern Adafruit_NeoPixel grnL;
extern Adafruit_NeoPixel bluL;
extern Adafruit_NeoPixel yelL;
extern Adafruit_NeoPixel cirL;
extern Adafruit_NeoPixel placL;
extern Metro fasterStripUpdateInterval;
extern EasyTransfer ET;
extern systemState inst;
extern ConcurrentAnimator animator;
extern AnimationConfig circleConfig;
extern AnimationConfig placardConfig;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  delay(500);
  randomSeed(analogRead(A5));

  Serial << F("Light startup.") << endl;

  ET.begin(details(inst), &Serial1);

  Serial << F("Total strip memory usage: ") << TOTAL_LED_MEM << F(" bytes.") << endl;
  Serial << F("Free RAM: ") << freeRam() << endl;

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  configureAnimations();

  Serial << F("Light: startup complete.") << endl;
}

void loop() {
  if (fasterStripUpdateInterval.check()) {
    Serial << inst.animation << endl;
    mapToAnimation(animator, inst);
    fasterStripUpdateInterval.reset();
  }

  static Metro quietUpdateInterval(10UL * 1000UL); // after 10 second of not instructions, we should do something.
  static byte lastPacketNumber=255;

  if (ET.receiveData() && inst.packetNumber != lastPacketNumber) {
    // track and apply deltas only
    lastPacketNumber = inst.packetNumber;

    //Serial << F("I. packetNumber") << lastPacketNumber << endl;

    //    for( byte i=0; i<N_COLORS; i++ ) {
    //      Serial << F(" Color ") << i << F("; R:") << inst.light[i].red << F(" G:") << inst.light[i].green << F(" B:") << inst.light[i].blue << endl;
    //    }

    // dispatch the requests to the buttons
    setStripColor(redL, inst.light[I_RED]);
    setStripColor(grnL, inst.light[I_GRN]);
    setStripColor(bluL, inst.light[I_BLU]);
    setStripColor(yelL, inst.light[I_YEL]);

    // toggle LED to ACK new button press
    static boolean ledStatus = false;
    ledStatus = !ledStatus;
    digitalWrite(LED_PIN, ledStatus);
    quietUpdateInterval.reset();
  }
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

