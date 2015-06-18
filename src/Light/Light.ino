// Compile for Mega

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// have: 2K memory, 3 bytes memory per LED, 60 LEDs per meter.
// so, we can support 2000/3/60=11.1 meters, other memory usage notwithstanding.

#include "Strip.h"

// MGD: please include #includes in top level .ino.
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
// TODO: move to NeoMatrix to take advantage of the 3x stacked strips
#include <Streaming.h>
#include <Metro.h>
#include <EasyTransfer.h>

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

#include "ConcurrentAnimator.h"
#include "AnimationConfig.h"
#include "Animations.h"
#include "ConcurrentAnimator.h"
#include "AnimateFunc.h"

extern Adafruit_NeoMatrix rimJob;
extern Adafruit_NeoPixel redL;
extern Adafruit_NeoPixel grnL;
extern Adafruit_NeoPixel bluL;
extern Adafruit_NeoPixel yelL;
extern Adafruit_NeoPixel cirL;
extern Adafruit_NeoPixel placL;
extern Metro fasterStripUpdateInterval;
extern EasyTransfer ET;
extern systemState inst;

void setup() {
  Serial.begin(115200);

  delay(500);

  Serial << F("Light startup.") << endl;

  Serial1.begin(115200);
  //start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
  ET.begin(details(inst), &Serial1);

  Serial << F("Total strip memory usage: ") << TOTAL_LED_MEM << F(" bytes.") << endl;
  Serial << F("Free RAM: ") << freeRam() << endl;

  // random seed from analog noise.
  randomSeed(analogRead(0));

  pinMode(LED_PIN, OUTPUT); digitalWrite(LED_PIN, LOW);

  Serial << F("RimJob: ") << RIM_N << F(" pixels in 4 segments of length ") << RIM_SEG_LENGTH << endl;
  Serial << F("RimJob: YEL starts at: ") << YEL_SEG_START << endl;
  Serial << F("RimJob: BLU starts at: ") << BLU_SEG_START << endl;
  Serial << F("RimJob: RED starts at: ") << RED_SEG_START << endl;
  Serial << F("RimJob: GRN starts at: ") << GRN_SEG_START << endl;

  // start
  rimJob.begin();
  redL.begin();
  grnL.begin();
  bluL.begin();
  yelL.begin();
  cirL.begin();
  placL.begin();

  Serial << F("Free RAM: ") << freeRam() << endl;

  Serial << F("Light: startup complete.") << endl;

}

void setupStrip(Adafruit_NeoPixel &strip, const uint32_t color) {
  Serial << F("Initializing strip with pixel count: ") << strip.numPixels() << endl;

  strip.begin();

  for ( uint16_t i = 0; i < strip.numPixels(); i++ ) {
    strip.setPixelColor(i, color);
  }

  strip.show();
}

void loop() {
  // update the strip automata on an interval
  /*
  if ( stripUpdateInterval.check() ) {

    // compute the next step and flag for show.
    updateRule90(rimJob, PIXEL_TTL); rimUpdated = true;

    // if we wanted the buttons to animate differently than the rim, this would be the place to do it.
    updateRule90(redL, PIXEL_TTL); redUpdated = true;
    updateRule90(grnL, PIXEL_TTL); grnUpdated = true;
    updateRule90(bluL, PIXEL_TTL); bluUpdated = true;
    updateRule90(yelL, PIXEL_TTL); yelUpdated = true;

    // if we wanted the middle chotsky to animate differently than the rim, this would be the place to do it.
    updateRule90(midL, PIXEL_TTL); midUpdated = true;

    stripUpdateInterval.reset();
  }
*/
  static Metro quietUpdateInterval(10UL *1000UL); // after 10 second of not instructions, we should do something.

  //check and see if a data packet has come in.
  if (ET.receiveData()) {

    Serial << F("I");

//    for( byte i=0; i<N_COLORS; i++ ) {
//      Serial << F(" Color ") << i << F("; R:") << inst.light[i].red << F(" G:") << inst.light[i].green << F(" B:") << inst.light[i].blue << endl;
//    }

    // dispatch the requests
    setStripColor(redL, inst.light[I_RED]);
    setStripColor(grnL, inst.light[I_GRN]);
    setStripColor(bluL, inst.light[I_BLU]);
    setStripColor(yelL, inst.light[I_YEL]);

    // toggle LED to ACK new button press
    static boolean ledStatus=false;
    ledStatus = !ledStatus;
    digitalWrite(LED_PIN, ledStatus);

    quietUpdateInterval.reset();
  }

  // when it's quiet, we need to do something with the LEDs
  if( quietUpdateInterval.check() ) {
    Serial << F("Quiet interval elapsed.") << endl;
    Serial << F("Free RAM=") << freeRam() << endl;
  }


}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

// unpack and pack functions
void extractColor(uint32_t c, uint8_t * cv) {
  cv[0] = (c << 8 ) >> 24;
  cv[1] = (c << 16) >> 24;
  cv[2] = (c << 24) >> 24;
}
// apprently, the color handling is slightly different btw matrix and strip?
uint32_t packColor(Adafruit_NeoPixel &strip, uint8_t * cv) {
  return ( strip.Color(cv[0], cv[1], cv[2]) );
}
uint32_t packColor(Adafruit_NeoMatrix &matrix, uint8_t * cv) {
  return ( matrix.Color(cv[0], cv[1], cv[2]) );
}
void printColor(uint32_t c) {
  // pull out RGB elements
  uint8_t cv[3];
  extractColor(c, cv);
  Serial << F(" R: ") << cv[0] << F(" G: ") << cv[1] << F(" B: ") << cv[2] << endl;
}

// adjust color with a decrement
uint32_t adjustColor(uint32_t c, unsigned long ttl) {
  // pull out RGB elements
  uint8_t cv[3];
  signed int cvl[3];
  extractColor(c, cv);
  int adj;

  // for each color
  for ( uint8_t i = 0; i < 3; i++ ) {

    // try to smoothly drop the colors
    if ( i == 0 ) adj = RED_MAX / (PIXEL_TTL / STRIP_UPDATE);
    else if ( i == 1 ) adj = GRN_MAX / (PIXEL_TTL / STRIP_UPDATE);
    else adj = BLU_MAX / (PIXEL_TTL / STRIP_UPDATE);

    if ( cv[i] >= adj + LED_OFF ) { // unsiged stuff.  take care.
      cv[i] = cv[i] - adj;
    } else {
      cv[i] = LED_OFF;
    }
    //    Serial << cv[i] << endl;
  }

  //  while(1);
  // repack and return
  return ( packColor(redL, cv) );
}

// merge colors with some kind of XOR
uint32_t mergeColor(uint32_t c1, uint32_t c2) {
  // use bitwise operations to get at packed color vector contents
  const uint8_t bitIndex[3] = {8, 16, 24};
  // store the color vector result
  uint8_t c1v[3], c2v[3], cv[3];

  // get the RGB components.
  extractColor(c1, c1v);
  extractColor(c2, c2v);

  // for each color
  for ( uint8_t i = 0; i < 3; i++ ) {
    // xor by color channel
    if ( c1v[i] > 0 && c2v[i] > 0 ) {
      // alive in both cells, so drop this channel.
      cv[i] = LED_OFF;
    } else {
      // dead in one, so sum them.  unsigned type is doing some funky stuff when >255.
      cv[i] = max(c1v[i], c2v[i]);
    }
  }
  return ( packColor(redL, cv) );
}

// updates the automata using a modified Rule 90
void updateRule90(Adafruit_NeoPixel &strip, unsigned long ttl) {
  // intialize first cell
  uint32_t cs = strip.getPixelColor(0);
  uint32_t ps = strip.getPixelColor(strip.numPixels() - 1);
  uint32_t ns, color;
  // store the color vector result
  uint8_t cv[3];

  // loop to update
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    // get next state
    if (i < strip.numPixels() - 1) {
      ns = strip.getPixelColor(i + 1);
    }
    else { // wrapping back around
      ns = strip.getPixelColor(0);
    }

    // apply Rule 90
    if ( ps == 0 ) {
      color = adjustColor(ns, ttl);
    } else if ( ns == 0 ) {
      color = adjustColor(ps, ttl);
    } else {
      color = mergeColor(adjustColor(ps, ttl), adjustColor(ns, ttl));
    }

    // set the pixel
    strip.setPixelColor(i, color);

    // next iteration setting
    ps = cs;
    cs = ns;
  }
}

void test(Adafruit_NeoPixel &strip) {
  // Some example procedures showing how to display to the pixels:
  colorWipe(strip, strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip, strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip, strip.Color(0, 0, 255), 50); // Blue

  // Send a theater pixel chase in...
  theaterChase(strip, strip.Color(127, 127, 127), 50); // White
  theaterChase(strip, strip.Color(127,   0,   0), 50); // Red
  theaterChase(strip, strip.Color(  0,   0, 127), 50); // Blue

  rainbow(strip, 20);
  rainbowCycle(strip, 20);
  theaterChaseRainbow(strip, 50);
}

// Fill the dots one after the other with a color
void colorWipe(Adafruit_NeoPixel &strip, uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(Adafruit_NeoPixel &strip, uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(strip, (i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(Adafruit_NeoPixel &strip, uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(strip, ((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(Adafruit_NeoPixel &strip, uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(Adafruit_NeoPixel &strip, uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel(strip, (i + j) % 255)); //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(Adafruit_NeoPixel &strip, byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void HSVtoRGB(int hue, int sat, int val, int *colors) {
  int r, g, b, base;

  // hue: 0-359, sat: 0-255, val (lightness): 0-255

  if (sat == 0) { // Achromatic color (gray).
    colors[0] = val;
    colors[1] = val;
    colors[2] = val;
  } else  {
    base = ((255 - sat) * val) >> 8;
    switch (hue / 60) {
      case 0:
        colors[0] = val;
        colors[1] = (((val - base) * hue) / 60) + base;
        colors[2] = base;
        break;
      case 1:
        colors[0] = (((val - base) * (60 - (hue % 60))) / 60) + base;
        colors[1] = val;
        colors[2] = base;
        break;
      case 2:
        colors[0] = base;
        colors[1] = val;
        colors[2] = (((val - base) * (hue % 60)) / 60) + base;
        break;
      case 3:
        colors[0] = base;
        colors[1] = (((val - base) * (60 - (hue % 60))) / 60) + base;
        colors[2] = val;
        break;
      case 4:
        colors[0] = (((val - base) * (hue % 60)) / 60) + base;
        colors[1] = base;
        colors[2] = val;
        break;
      case 5:
        colors[0] = val;
        colors[1] = base;
        colors[2] = (((val - base) * (60 - (hue % 60))) / 60) + base;
        break;
    }

  }
}


// number, twinkle color, background color, delay
// twinkleRand(5,strip.Color(255,255,255),strip.Color(255, 0, 100),90);

// twinkle random number of pixels
void twinkleRand(Adafruit_NeoPixel &strip, int num, uint32_t c, uint32_t bg) {
  // set background
  //	 stripSet(bg,0);
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, bg);
  }
  // for each num
  for (int i = 0; i < num; i++) {
    strip.setPixelColor(random(strip.numPixels()), c);
  }
}

// other options for effects at: http://funkboxing.com/wordpress/wp-content/_postfiles/sk_qLEDFX_POST.ino

void setStripColor(Adafruit_NeoPixel &strip, uint32_t c) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}
void setStripColor(Adafruit_NeoPixel &strip, colorInstruction &inst) {
  setStripColor(strip, strip.Color(inst.red, inst.green, inst.blue) );
}

void setStripColor(Adafruit_NeoMatrix &matrix, uint32_t c) {
  for (int i = 0; i < matrix.numPixels(); i++) {
    matrix.setPixelColor(i, c);
  }
  matrix.show();
}

