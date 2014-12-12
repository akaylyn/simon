// Compile for Arduino Pro or Pro Mini

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#include <Adafruit_NeoPixel.h>
#include <Streaming.h>
#include <Metro.h>
#include <Bounce.h>

// RIM of LEDs
#define RIM_PIN A3 // wire to rim DI pin.  Include a 330 Ohm resistor in series.
#define RIM_N 85 // best if divisible by 4
#define RIM_LED_TTL 3000UL // reduce intensity at each update, so this is the amount of time we can expect a pixel to last
#define RIM_UPDATE 100UL // update the automata on the rim at this interval
#define RIM_ADD_PIXEL RIM_LED_TTL/3 // since the pixels have a TTL, we need to add some when there's nothing going on.
#define RED_SEG_OFFSET floor((float)RIM_N / 4) // sort out where the LEDs are that match the button locations, in software.  You're welcome.
// LED indicator to ack button presses
#define LED_PIN 13

// button pins.  wire to Mega GPIO, bring LOW to indicate pressed.
#define RED_BUTTON 4
#define YEL_BUTTON 5
#define GRN_BUTTON 6
#define BLU_BUTTON 7
#define DEBOUNCE_TIME 3UL
Bounce redButton = Bounce( RED_BUTTON, DEBOUNCE_TIME );
Bounce grnButton = Bounce( GRN_BUTTON, DEBOUNCE_TIME );
Bounce bluButton = Bounce( BLU_BUTTON, DEBOUNCE_TIME );
Bounce yelButton = Bounce( YEL_BUTTON, DEBOUNCE_TIME );

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel rimJob = Adafruit_NeoPixel(RIM_N, RIM_PIN, NEO_GRB + NEO_KHZ800);

// define some colors.
#define RED_MAX 255
#define GRN_MAX 220
#define BLU_MAX 210
#define LED_OFF 0 // makes it a litle brighter in the Console.  0 is fine, too.
const uint32_t SweetLoveMakin = rimJob.Color(RED_MAX / 4, GRN_MAX / 6, BLU_MAX / 9);
const uint32_t Red = rimJob.Color(RED_MAX, LED_OFF, LED_OFF);
const uint32_t Yel = rimJob.Color(RED_MAX, GRN_MAX, LED_OFF);
const uint32_t Grn = rimJob.Color(LED_OFF, GRN_MAX, LED_OFF);
const uint32_t Blu = rimJob.Color(LED_OFF, LED_OFF, BLU_MAX);
const uint32_t Dead = rimJob.Color(LED_OFF, LED_OFF, LED_OFF);

void setup() {
  Serial.begin(115200);

  randomSeed(analogRead(0));
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(RED_BUTTON, INPUT_PULLUP);
  pinMode(GRN_BUTTON, INPUT_PULLUP);
  pinMode(YEL_BUTTON, INPUT_PULLUP);
  pinMode(BLU_BUTTON, INPUT_PULLUP);
  
  // Initialize all pixels to 'sweet love makin'
  rimJob.begin();
  for ( uint16_t i = 0; i < rimJob.numPixels(); i++ ) {
    rimJob.setPixelColor(i, SweetLoveMakin);
  }
  rimJob.show();
  delay(1000);

  //  test(rimJob); // run through some tests
  Serial << F("Light: startup complete.") << endl;
}

void loop() {

  // update the rimJob
  static Metro rimJobUpdate(RIM_UPDATE);
  if ( rimJobUpdate.check() ) {
    // compute the next step
    updateRule90(rimJob, RIM_LED_TTL);

    // go to press
    if( rimJob.canShow() ) rimJob.show();
    rimJobUpdate.reset();
  }
  
  // when it's quiet, add some pixels
  static Metro quietHeartBeat(RIM_ADD_PIXEL);
  if ( quietHeartBeat.check() ) {
    quietHeartBeat.reset();
    uint32_t color;
    switch(random(0,4)) {
      case 0: color=Red; break;
      case 1: color=Grn; break;
      case 2: color=Blu; break;
      case 3: color=Yel; break;
    }
    rimJob.setPixelColor(random(0, RIM_N), color);
  }
  
  // check for button pressed
  redButton.update();
  grnButton.update();
  bluButton.update();
  yelButton.update();
  if ( redButton.read() == LOW ) {
    buttonPressPattern(0);
    if( rimJob.canShow()) rimJob.show();
    quietHeartBeat.reset();
    digitalWrite(LED_PIN, HIGH);
  } else if ( grnButton.read() == LOW ) {
    buttonPressPattern(1);
    if( rimJob.canShow()) rimJob.show();
    quietHeartBeat.reset();
    digitalWrite(LED_PIN, HIGH);
  } else if ( bluButton.read() == LOW ) {
    buttonPressPattern(2);
    if( rimJob.canShow()) rimJob.show();
    quietHeartBeat.reset();
    digitalWrite(LED_PIN, HIGH);
  } else if ( yelButton.read() == LOW ) {
    buttonPressPattern(3);
    if( rimJob.canShow()) rimJob.show();
    quietHeartBeat.reset();
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

}

void buttonPressPattern(uint8_t button) {
  const uint16_t rimJobSegLength = floor((float)RIM_N / 4);

  switch (button) {
    case 0: buttonPressToRim(Red, RED_SEG_OFFSET, rimJobSegLength); break;
    case 2: buttonPressToRim(Blu, RED_SEG_OFFSET + rimJobSegLength, rimJobSegLength); break;
    case 3: buttonPressToRim(Yel, RED_SEG_OFFSET + rimJobSegLength * 2, rimJobSegLength); break;
    case 1: buttonPressToRim(Grn, RED_SEG_OFFSET + rimJobSegLength * 3, rimJobSegLength); break;
  }

}

void buttonPressToRim(const uint32_t color, uint16_t segStart, uint16_t segLength) {
  Serial << F("Button.  Adding pixels from ") << segStart << F(" to ") << segStart + segLength - 1 << F(". Color: ");
  printColor(color);

  // clear the segment and lay down this color
  for ( int i = 1; i < segLength; i++ )  {
    rimJob.setPixelColor((segStart + i) % RIM_N, color);
  }

}

// unpack and pack functions
void extractColor(uint32_t c, uint8_t * cv) {
  cv[0] = (c << 8 ) >> 24;
  cv[1] = (c << 16) >> 24;
  cv[2] = (c << 24) >> 24;
}
uint32_t packColor(uint8_t * cv) {
  return ( rimJob.Color(cv[0], cv[1], cv[2]) );
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
    if( i==0 ) adj=RED_MAX/(RIM_LED_TTL/RIM_UPDATE);
    else if( i==1 ) adj=GRN_MAX/(RIM_LED_TTL/RIM_UPDATE);
    else adj=BLU_MAX/(RIM_LED_TTL/RIM_UPDATE);
    
    if ( cv[i] >= adj + LED_OFF ) { // unsiged stuff.  take care.
      cv[i] = cv[i] - adj;
    } else {
      cv[i] = LED_OFF;
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
      cv[i] = LED_OFF;
    } else {
      // dead in one, so sum them.  unsigned type is doing some funky stuff when >255.
      cv[i] = max(c1v[i], c2v[i]);
    }
  }
  return ( packColor(cv) );
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
    if ( ps == Dead ) {
      color = adjustColor(ns, ttl);
    } else if ( ns == Dead ) {
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

