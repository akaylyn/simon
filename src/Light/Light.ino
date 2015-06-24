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

// tracks the cycles
typedef struct {
  boolean live; // live or dead
  uint32_t x,y; // location
  uint32_t color; // color of the cycle
  byte movePref; // CW or CCW preference
} cycle_t;

#define MAX_CYCLES 20
cycle_t cycles[MAX_CYCLES];

#define CYCLE_DICK_MOVE_PERCENT 15 // chance that a cycle will throw some "zigs" in it's path around the rim
#define CYCLE_TRAIL_LENGTH BLU_X-RED_X // a light cycle leaves a trail as long as 1/4 of the rim circumference

#define ADD_INTERVAL 1000UL
Metro addCycles(ADD_INTERVAL);

// cycle colors
const uint32_t Red = rimJob.Color(RED_MAX, LED_OFF, LED_OFF);
const uint32_t Yel = rimJob.Color(RED_MAX, GRN_MAX, LED_OFF);
const uint32_t Grn = rimJob.Color(LED_OFF, GRN_MAX, LED_OFF);
const uint32_t Blu = rimJob.Color(LED_OFF, LED_OFF, BLU_MAX);
const uint32_t Dead = rimJob.Color(LED_OFF, LED_OFF, LED_OFF);
const uint32_t White = rimJob.Color(RED_MAX, GRN_MAX, BLU_MAX);

void setup() {
  Serial.begin(115200);

  delay(500);

  // random seed set from electrical noise on an analog pin.
  randomSeed(analogRead(A5));

  Serial << F("Light startup.") << endl;

  Serial1.begin(115200);
  //start the library, pass in the data details and the name of the serial port.
  // Can be Serial, Serial1, Serial2, etc.
  ET.begin(details(inst), &Serial1);

  Serial << F("Total strip memory usage: ") << TOTAL_LED_MEM << F(" bytes.") << endl;
  Serial << F("Free RAM: ") << freeRam() << endl;

  // random seed from analog noise.
  randomSeed(analogRead(0));

  pinMode(LED_PIN, OUTPUT); digitalWrite(LED_PIN, LOW);

  // start
  configureAnimations();

  // clear cycles
  for( int c=0; c<MAX_CYCLES; c++ ) {
    cycles[c].live=false;
  }

  rimJob.begin();
  addCycle(RED_X, ALL_Y, Red);
  addCycle(GRN_X, ALL_Y, Grn);
  addCycle(BLU_X, ALL_Y, Blu);
  addCycle(YEL_X, ALL_Y, Yel);
  rimJob.show();

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
int gameplayCounter = 0;
const int maxGameplayCounter = 100;

void loop() {
  if (fasterStripUpdateInterval.check()) {
    // animation instructions
    mapToAnimation(animator, inst);

    fasterStripUpdateInterval.reset();
    animator.animate(rainbowGlow, circleConfig);

    // MGD: added a manual animation on center coaster and placard
    //rainbowUpdate(cirL);
    rainbowUpdateReverse(placL);
    // MGD: I assume there's a way to do something like this with the Animation.cpp... but I couldn't figure out how to do that.
    //      maybe take the animations that are at the bottom of this .ino and transfer them as a "pallette" to work from?

    // Tron light cycles on the rim.
    fadeCycles();
    moveCycles();
//    serialPrint();
    rimJob.show();

  }

  static Metro quietUpdateInterval(10UL * 1000UL); // after 10 second of not instructions, we should do something.

  static byte lastPacketNumber=255;
  //check and see if a data packet has come in.
  if (ET.receiveData() && inst.packetNumber != lastPacketNumber) {
    // track and apply deltas only
    lastPacketNumber = inst.packetNumber;

    Serial << F("I. packetNumber") << lastPacketNumber << endl;


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

    // dispatch the requests to the rim
    if( inst.light[I_RED].red > 0 ) addCycle(RED_X, ALL_Y, Red);
    if( inst.light[I_GRN].green > 0 ) addCycle(GRN_X, ALL_Y, Grn);
    if( inst.light[I_BLU].blue > 0 ) addCycle(BLU_X, ALL_Y, Blu);
    if( inst.light[I_YEL].red > 0 && inst.light[I_YEL].green > 0 ) addCycle(YEL_X, ALL_Y, Yel);

    quietUpdateInterval.reset();
    addCycles.reset();
  }

  // when it's quiet, we need to do something with the LEDs
  if ( addCycles.check() ) {
    Serial << F("Quiet interval elapsed.") << endl;
    Serial << F("Free RAM=") << freeRam() << endl;

    // add some light cycles
    switch( random(0,4) ) {
      case 0: addCycle(RED_X, ALL_Y, Red); break;
      case 1: addCycle(GRN_X, ALL_Y, Grn); break;
      case 2: addCycle(BLU_X, ALL_Y, Blu); break;
      case 3: addCycle(YEL_X, ALL_Y, Yel); break;
    }

  }

  /*
TODO: fix this and put it in a metro loop
  // compute the next step and flag for show.
  updateRule90(rimJob, PIXEL_TTL); rimUpdated = true;

  // if we wanted the buttons to animate differently than the rim, this would be the place to do it.
  updateRule90(redL, PIXEL_TTL); redUpdated = true;
  updateRule90(grnL, PIXEL_TTL); grnUpdated = true;
  updateRule90(bluL, PIXEL_TTL); bluUpdated = true;
  updateRule90(yelL, PIXEL_TTL); yelUpdated = true;

  // if we wanted the middle chotsky to animate differently than the rim, this would be the place to do it.
  updateRule90(midL, PIXEL_TTL); midUpdated = true;
  */


}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
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
void rainbowUpdate(Adafruit_NeoPixel &strip) {
  static byte colorPos = 0;

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(strip, (byte)((i + colorPos) % 255) ) );
  }
  strip.show();
  colorPos+=random(1,5); // increment for next pass
}
void rainbowUpdateReverse(Adafruit_NeoPixel &strip) {
  static byte colorPos = 0;

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(strip, (byte)((i + colorPos) % 255) ) );
  }
  strip.show();
  colorPos-=random(1,5); // increment for next pass
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

//Theatre-style crawling lights.
void theaterUpdate(Adafruit_NeoPixel &strip) {
  static byte colorPos = 0;

  // turn them all off
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(strip, colorPos));
  }

  strip.show();
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

//***************** Tron light cycle code:


void serialPrint() {
  static unsigned long it=0;
  Serial << endl << it << F(":") << endl;
  for( int y=0; y<RIM_Y; y++) {
    for( int x=0; x<RIM_X; x++) {
      uint32_t c = rimJob.getPixelColor(getPixelN(x,y));
      boolean isC = isCycle(x,y);
      char p;

           if( isC && c==Red ) p = 'R';
      else if( isC && c==Grn ) p = 'G';
      else if( isC && c==Blu ) p = 'B';
      else if( isC && c==Yel ) p = 'Y';
      else if( isC ) p = '!'; // BAD: cycles have a color
      else if( c==Red ) p = 'r';
      else if( c==Grn ) p = 'g';
      else if( c==Blu ) p = 'b';
      else if( c==Yel ) p = 'y';
      else if( c==Dead ) p = ' ';
      else if( c==White ) p = '*';
      else p = '.'; // faded/cycle trail

      Serial << p;
    }
    Serial << endl;
  }

  it++;
}

// looks for active cycle at a pixel location
boolean isCycle(int x, int y) {
  for( int c=0; c<MAX_CYCLES; c++ ) {
    if( cycles[c].live && cycles[c].x==x && cycles[c].y==y )
      return( true );
  }
  return( false );
}

void addCycle(uint32_t x, uint32_t y, uint32_t color) {

  byte availableCycle=0;
  while( cycles[availableCycle].live ) {
    availableCycle++;
    if( availableCycle == MAX_CYCLES ) {
      Serial << F("addCycle error.  no available cycle slots.  Halting!") << endl;
      while(1);
    }
  }

  rimJob.setPixelColor(getPixelN(x,y), color);

  cycles[availableCycle].x = x;
  cycles[availableCycle].y = y;
  cycles[availableCycle].color = color;
  cycles[availableCycle].live = true;

  static byte movePref = 0;
  (++movePref) %= 2;
  cycles[availableCycle].movePref = movePref; // likes to go CW or CCW, generally

}

void fadeCycles() {
  const int fadeAmount = 255 / (CYCLE_TRAIL_LENGTH);

  for( int p=0; p<rimJob.numPixels(); p++ ) {
    uint32_t c = rimJob.getPixelColor(p);

    // assuming RGB order.
    byte red = constrain( (int)((c << 8) >> 24) - fadeAmount, 0, 255);
    byte green = constrain( (int)((c << 16) >> 24) - fadeAmount, 0, 255);
    byte blue = constrain( (int)((c << 24) >> 24) - fadeAmount, 0, 255);

    rimJob.setPixelColor(p, red, green, blue);
  }
}

void moveCycles() {
  for( byte c=0; c<MAX_CYCLES; c++ ) {
    if( cycles[c].live ) {
      moveThisCycle(c);
     }
  }
}

void moveThisCycle(byte c) {
  int cw = (int)cycles[c].x+1;
  if( cw<0 ) cw = RIM_X-1;
  else if ( cw >= RIM_X) cw = 0;

  int ccw = (int)cycles[c].x-1;
  if( ccw<0 ) ccw = RIM_X-1;
  else if ( ccw >= RIM_X) ccw = 0;

  uint32_t moveX[4] = {
    cw, // CW. wrap.
    ccw, // CCW. wrap
    cycles[c].x, // DOWN
    cycles[c].x, // UP
  };
  uint32_t moveY[4] = {
    cycles[c].y, // CW
    cycles[c].y, // CCW
    constrain(((int)cycles[c].y-1), 0, RIM_Y-1), // DOWN.  no wrap.
    constrain(((int)cycles[c].y+1), 0, RIM_Y-1), // UP. no wrap.
  };

  // drivin' and cryin'

  // preference for moves
  byte movePref[4];
  byte r = random(0,100);
  // add some randomness to there's "jogs" in the paths.
  if( cycles[c].y == ALL_Y && r >= CYCLE_DICK_MOVE_PERCENT ) {
    // if we're in the center, try CW/CCW frist
    movePref[0]=cycles[c].movePref; // either CW or CCW.
    movePref[1]= movePref[0]==0 ? 1 : 0;
    // and only move UP or DOWN if we have to
    movePref[2]=random(2,4); // either UP or DOWN.
    movePref[3]= movePref[2]==2 ? 3 : 2;
  } else {
    // try to move UP or DOWN
    movePref[0]=random(2,4); // either UP or DOWN.
    movePref[1]= movePref[0]==2 ? 3 : 2;
    // and only move CW/CCW if we have to
    movePref[2]=cycles[c].movePref; // either CW or CCW.
    movePref[3]= movePref[2]==0 ? 1 : 0;
  }

  // try some moves
  for( byte m=0; m<4; m++ ) {
    if( rimJob.getPixelColor(getPixelN(moveX[movePref[m]], moveY[movePref[m]])) == Dead ) {
      // good.
      cycles[c].x = moveX[movePref[m]];
      cycles[c].y = moveY[movePref[m]];

      rimJob.setPixelColor(getPixelN(cycles[c].x,cycles[c].y), cycles[c].color);

      return;
    }
  }
  // uh oh. smash!
  cycles[c].live = false;
  rimJob.setPixelColor(getPixelN(cycles[c].x,cycles[c].y), White);
}

// returns the i-th pixel mapped to x,y
uint32_t getPixelN(uint32_t x, uint32_t y) {
  if( x >= RIM_X || y >= RIM_Y ) {
    Serial << F("getPixelN error: out of range.  x=") << x << F(" y=") << y << endl << F("Halting.") << endl;
    while(1);
  }
  return( y*RIM_X + x ); // this could be right?
}

