#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <Streaming.h>
#include <Metro.h>
#include <EasyTransfer.h>
#include <avr/wdt.h> // watchdog timer
#include <Simon_Common.h> // common message definition

#include "Strip.h"
#include "AnimationConfig.h"
#include "Animations.h"
#include "ConcurrentAnimator.h"
#include "AnimateFunc.h"

#define PIN 4

//extern Animations animations;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(49, PIN, NEO_GRB + NEO_KHZ800);
extern Adafruit_NeoMatrix rimJob;
extern Adafruit_NeoPixel redL;
extern Adafruit_NeoPixel grnL;
extern Adafruit_NeoPixel bluL;
extern Adafruit_NeoPixel yelL;
extern Metro fasterStripUpdateInterval;

extern ConcurrentAnimator animator;
extern AnimationConfig redButtonConfig;
extern AnimationConfig greenButtonConfig;
extern AnimationConfig blueButtonConfig;
extern AnimationConfig yellowButtonConfig;
extern AnimationConfig rimConfig;
extern AnimationConfig circleConfig;
extern AnimationConfig placardConfig;

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
/*
void setStripColor(Adafruit_NeoPixel *strip, int r, int g, int b) {
  for (int i = 0; i < strip->numPixels(); i++) {
    strip->setPixelColor(i, strip->Color(r, g, b));
  }
  strip->show();
}
*/

void setup() {
  Serial.begin(115200);
  configureAnimations();
  //setStripColor(rimJob, LED_OFF, LED_OFF, LED_OFF);
  Serial << "Setup Complete" << endl;
}

/******************************************************************************
 * Main Loop
 ******************************************************************************/
void loop() {
  if (fasterStripUpdateInterval.check()) {
    /*
    animator.animate(colorWipeMatrix, rimConfig);
    animator.animate(laserWipe, redButtonConfig);
    animator.animate(laserWipe, greenButtonConfig);
    animator.animate(laserWipe, blueButtonConfig);

    animator.animate(colorWipe, yellowButtonConfig);

    animator.animate(colorWipe, circleConfig);
    animator.animate(colorWipe, placardConfig);
    */

    animator.animate(gameplayMatrix, rimConfig);

    fasterStripUpdateInterval.reset();
  }
}

