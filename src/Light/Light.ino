#include <Adafruit_NeoPixel.h>
#include <Streaming.h>
#include <Metro.h>

// RIM of LEDs
#define RIM_PIN 6 // wire to rim DI pin.  Include a 330 Ohm resistor in series.
#define RIM_N 85 // best if divisible by 4
#define RIM_DECAY_RATE 1 // reduce intensity at each update
#define RIM_UPDATE 100UL // update the automata on the rim at this interval

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel rim = Adafruit_NeoPixel(RIM_N, RIM_PIN, NEO_GRB + NEO_KHZ800);

// define some colors.
const uint32_t Red = rim.Color(255, 0, 0);
const uint32_t Grn = rim.Color(0, 255, 0);
const uint32_t Blu = rim.Color(0, 0, 255);
const uint32_t Yel = rim.Color(128, 128, 0);
const uint32_t Dead = rim.Color(0, 0, 0);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  Serial.begin(115200);

  randomSeed(analogRead(0));

  rim.begin(); // Initialize all pixels to 'off'
  simulateButtonPress();
  rim.show();
  //  test(rim); // run through some tests
  Serial << "Light: startup complete." << endl;
}

void loop() {
  // simulate a button press
  // simulate button presses.  replace this with Button library to 4x GPIO pins.
  static Metro buttonUpdate(1000UL);
  if ( buttonUpdate.check() ) {
    simulateButtonPress();
    rim.show();
    buttonUpdate.reset();
  }

  // update the rim
  static Metro rimUpdate(RIM_UPDATE);
  if ( rimUpdate.check() ) {
    Serial << ".";
    updateRule90(rim, RIM_DECAY_RATE);
    rim.show();
    rimUpdate.reset();
  }

}

void simulateButtonPress() {
  const uint16_t rimSegLength = floor((float)RIM_N / 4);

  uint8_t button = random(0, 4); // pick a button
  switch (button) {
      delay(1000);
    case 0: buttonPressToRim(Red, 0, rimSegLength); break;
    case 2: buttonPressToRim(Blu, rimSegLength, rimSegLength); break;
    case 3: buttonPressToRim(Yel, rimSegLength * 2, rimSegLength); break;
    case 1: buttonPressToRim(Grn, rimSegLength * 3, rimSegLength); break;
  }

}

void buttonPressToRim(const uint32_t color, uint16_t segStart, uint16_t segLength) {
  Serial << F("Button.  Adding pixels from ") << segStart << F(" to ") << segStart + segLength - 1;
  printColor(color);
  Serial << endl;

  // clear the segment
  for ( uint16_t i = segStart + 1; i < segLength; i++ ) {
    rim.setPixelColor(segStart + i, Dead);
  }
  // put down the color every other pixel
  for ( uint16_t i = 0; i < segLength; i += 3 ) {
    rim.setPixelColor(segStart + i, color);
  }

}

// unpack and pack functions
void extractColor(uint32_t c, uint8_t * cv) {
  cv[0] = (c << 8 ) >> 24;
  cv[1] = (c << 16) >> 24;
  cv[2] = (c << 24) >> 24;
}
uint32_t packColor(uint8_t * cv) {
  return ( rim.Color(cv[0], cv[1], cv[2]) );
}
void printColor(uint32_t c) {
  // pull out RGB elements
  uint8_t cv[3];
  extractColor(c, cv);
  Serial << " R: " << cv[0] << " G: " << cv[1] << " B: " << cv[2] << endl;
}

// adjust color with a decrement
uint32_t adjustColor(uint32_t c, int adj) {
  // pull out RGB elements
  uint8_t cv[3];
  signed int cvl[3];
  extractColor(c, cv);

  // for each color
  for ( uint8_t i = 0; i < 3; i++ ) {
    if( cv[i]>= adj ) { // unsiged stuff.  take care.
      cv[i] = cv[i] - adj;
    } else {
      cv[i] = 0;
    }
//    Serial << cv[i] << endl;
  }
  
//  while(1);
  // repack and return
  return ( packColor(cv) );
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
      cv[i] = 0;
    } else {
      // dead in one, so sum them.
      cv[i] = c1v[i] + c2v[i];
    }
  }
  return ( packColor(cv) );
}

// updates the automata using a modified Rule 90
void updateRule90(Adafruit_NeoPixel &strip, uint8_t decay) {
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
    if ( ps == Dead ) {
      color = adjustColor(ns, decay);
    } else if ( ns == Dead ) {
      color = adjustColor(ps, decay);
    } else {
      color = adjustColor(mergeColor(ps, ns), decay);
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

