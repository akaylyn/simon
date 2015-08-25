#include "Animations.h"

void setStripColor(Adafruit_NeoPixel &strip, int r, int g, int b) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void setStripColor(Adafruit_NeoPixel &strip, uint32_t c) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}
void setStripColor(Adafruit_NeoPixel &strip, colorInstruction &inst) {
  strip.setBrightness(255);
  setStripColor(strip, strip.Color(inst.red, inst.green, inst.blue) );
}

void setStripColor(Adafruit_NeoMatrix &matrix, uint32_t c) {
  for (int i = 0; i < matrix.numPixels(); i++) {
    matrix.setPixelColor(i, c);
  }
  matrix.show();
}


void colorWipeMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData) {
  int* pos = (int*) posData;
  int next = (*pos);

  if (next > matrix.width()) {
    next = 0;
    setStripColor((Adafruit_NeoPixel&) matrix, LED_OFF, LED_OFF, LED_OFF);
  } else {
    ++next;
  }
  matrix.drawPixel(next, 0, matrix.Color(r,g,b));
  matrix.drawPixel(next, 1, matrix.Color(r,g,b));
  matrix.drawPixel(next, 2, matrix.Color(r,g,b));
  (*pos) = next;
}

// Fill the dots one after the other with a color
// returns the position of the led that was lit
void colorWipe(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData) {
  int* pos = (int*) posData;
  int next = (*pos);

  if (next > strip.numPixels()) {
    next = 0;
  } else {
    ++next;
  }

  strip.setPixelColor(next, strip.Color(r, g, b));
  (*pos) = next;
}

// color wipes the last 8 pixels on the buttons
void laserWipe(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData) {
  LaserWipePosition* pos = static_cast<LaserWipePosition*>(posData);

  // next is relative to the previous position
  int next = pos->prev;
  int end = strip.numPixels() - 1;
  int start = end - 7;

  // first pixel on, direction set
  if (pos->prev == 0) {
    next = start;
  }

  // reverse direction if at an edge
  if (pos->prev == end) {
    pos->dir = 1;
  }
  else if (pos->prev == start) {
    pos->dir = 0;
  }

  // proceed to the next position
  if (pos->dir == 1) {
    --next;
  } else {
    ++next;
  }

  // clear out the last color and set the next one
  strip.setPixelColor(pos->prev, strip.Color(LED_OFF, LED_OFF, LED_OFF));
  strip.setPixelColor(next, strip.Color(r, g, b));
  pos->prev = next;
}

// IN PROGRESS
// color wipes the outside of the rim
void laserWipeEdge(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData) {
  LaserWipePosition* pos = static_cast<LaserWipePosition*>(posData);

  // next is relative to the previous position
  int next = pos->prev;
  int end = end + 31;
  int start = 9;

  if (next == end) {
    next++;
  }
  else if (next > end + 2 || next < end) {
    ++next;
    strip.setPixelColor(pos->prev, strip.Color(LED_OFF, LED_OFF, LED_OFF));
    strip.setPixelColor(next, strip.Color(r, g, b));
    pos->prev = next;
  }

  // clear out the last color and set the next one
}

void rainbowGlow(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData) {
  //static byte colorPos = 0;
  int* pos = (int*) posData;
  int next = (*pos);

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(strip, (byte)((i + next) % 255) ) );
  }
  //strip.show();

  next+=random(1,5); // increment for next pass
  (*pos) = next;
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

// Proximity Pulse Matrix, used in Proximity Mode
void proximityPulseMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData) {
  ProxPulsePosition* pos = static_cast<ProxPulsePosition*>(posData);

  uint32_t ledOff = matrix.Color(LED_OFF, LED_OFF, LED_OFF);
  int maxTailLength = 10;

  // clear
  if (pos->magnitude < 25) {
    if (pos->tailLength > maxTailLength) {
      int end = pos->prev - maxTailLength;
      if (pos->prev == 0 || pos->prev < maxTailLength) {
        end = matrix.width() + end;
      }
      matrix.drawPixel(end, 0, ledOff);
      matrix.drawPixel(end, 1, ledOff);
      matrix.drawPixel(end, 2, ledOff);
    }
  }

  pos->prev += 1;
  pos->tailLength+=1;

  if (pos->prev > matrix.width()) {
    pos->prev = 0;
  }

  matrix.drawPixel(pos->prev, 0, matrix.Color(r,g,b));
  matrix.drawPixel(pos->prev, 1, matrix.Color(r,g,b));
  matrix.drawPixel(pos->prev, 2, matrix.Color(r,g,b));

  //Serial << r << " " << g << " " << b << " " << pos->magnitude << " " << pos->prev << endl;
}

// Gameplay idle mode
void idleMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData) {
  ProxPulsePosition* pos = static_cast<ProxPulsePosition*>(posData);
  if (pos->prev == 3) pos->prev = 0;
  if (pos->magnitude == 0) pos->magnitude = 255;

  matrix.setBrightness(100);

  uint32_t randomColor = random(255);
  for (int i = 0; i < matrix.width(); i++) {
    matrix.drawPixel(i, pos->prev, Wheel(matrix, randomColor));
  }

  pos->prev += 1;
  pos->magnitude -= 1;
}

void twinkleRand(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.setPixelColor(random(strip.numPixels()), random(255));
}

/*****************************************************************************/
// Gameplay animation
static int gameplayMaxWidth = 13;
static int RedMidPoint = 7;
static int BlueMidPoint = 34;
static int YellowMidPoint = 61;
static int GreenMidPoint = 88;
void gameplayMatrix(Adafruit_NeoMatrix &matrix, int r, int g, int b, void *posData) {
  matrix.setBrightness(50);
  uint32_t ledOff = matrix.Color(LED_OFF, LED_OFF, LED_OFF);
  GameplayPosition* pos = static_cast<GameplayPosition*>(posData);

  //Serial << r << " " << g << " " << b << " " << pos->yellow << " " << pos->decayPos->prev << endl;

  if (r == 255) { // 7
    gameplayFillFromMiddle(matrix, RedMidPoint, pos->redOffset, matrix.Color(255, 0, 0));
    pos->redOffset = getFilledOffset(pos->redOffset);
  }
  else {
    gameplayDecayMatrix(matrix, pos, 1, 0, 0, 0);
  }

  if (b == 255) { // 34
    gameplayFillFromMiddle(matrix, BlueMidPoint, pos->blueOffset, matrix.Color(0, 0, 255));
    pos->blueOffset = getFilledOffset(pos->blueOffset);
  }
  else {
    gameplayDecayMatrix(matrix, pos, 0, 0, 1, 0);
  }

  if (pos->yellow == 255) { // 61
    gameplayFillFromMiddle(matrix, YellowMidPoint, pos->yellowOffset, matrix.Color(255, 255, 0));
    pos->yellowOffset = getFilledOffset(pos->yellowOffset);
  }
  else {
    gameplayDecayMatrix(matrix, pos, 0, 0, 0, 1);
  }

  if (g == 255) { // 88
    gameplayFillFromMiddle(matrix, GreenMidPoint, pos->greenOffset, matrix.Color(0, 255, 0));
    pos->greenOffset = getFilledOffset(pos->greenOffset);
  }
  else {
    gameplayDecayMatrix(matrix, pos, 0, 1, 0, 0);
  }
}

void gameplayDecayMatrix(Adafruit_NeoMatrix &matrix, void *posData, bool r, bool g, bool b, bool y) {
  GameplayPosition* pos = static_cast<GameplayPosition*>(posData);
  uint32_t ledOff = matrix.Color(LED_OFF, LED_OFF, LED_OFF);

  if (r) {
    gameplayFillFromMiddle(matrix, RedMidPoint, pos->redOffset, ledOff);
    pos->redOffset = getDecayedOffset(pos->redOffset);
  }

  if (g) {
    gameplayFillFromMiddle(matrix, GreenMidPoint, pos->greenOffset, ledOff);
    pos->greenOffset = getDecayedOffset(pos->greenOffset);
  }

  if (b) {
    gameplayFillFromMiddle(matrix, BlueMidPoint, pos->blueOffset, ledOff);
    pos->blueOffset = getDecayedOffset(pos->blueOffset);
  }

  if (y) {
    gameplayFillFromMiddle(matrix, YellowMidPoint, pos->yellowOffset, ledOff);
    pos->yellowOffset = getDecayedOffset(pos->yellowOffset);
  }
}

int getFilledOffset(int offset) {
  offset += 1;
  if (offset >= gameplayMaxWidth) {
    offset = gameplayMaxWidth;
  }
  return offset;
}

int getDecayedOffset(int offset) {
  offset -= 1;
  if (offset < 0) {
    offset = 0;
  }
  return offset;
}

void gameplayFillFromMiddle(Adafruit_NeoMatrix &matrix, int center, int prev, uint16_t color) {
  int left = center-prev;
  int right = center+prev;

  if (left < 0)
    left = matrix.width() + left;

  for (int height = 0; height < 3; height++) {
    matrix.drawPixel(left, height, color);
    matrix.drawPixel(right, height, color);
  }
}

/*****************************************************************************/
// Tron light cycle code:

void tronLightCycles(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData) {
  TronPosition* data = static_cast<TronPosition*>(posData);
  // dispatch the requests to the rim. number of cycles is proportional to the light level at each button
  if (data->addCycle)
    addCycle(strip, data->cycles, data->x, data->y, strip.Color(r, g, b));
  //addCycles.reset();
  moveCycles(strip, data->cycles);
  fadeCycles(strip);
}

// looks for active cycle at a pixel location
boolean isCycle(TronCycles *cycles, int x, int y) {
  for( int c=0; c<MAX_CYCLES; c++ ) {
    if( cycles[c].live && cycles[c].x==x && cycles[c].y==y )
      return( true );
  }
  return( false );
}

void addCycle(Adafruit_NeoPixel &strip, TronCycles *cycles, uint32_t x, uint32_t y, uint32_t color) {

  byte availableCycle=0;
  while( cycles[availableCycle].live ) {
    availableCycle++;
    if( availableCycle == MAX_CYCLES ) {
      Serial << F("addCycle error.  no available cycle slots.  Halting!") << endl;
      while(1);
    }
  }

  strip.setPixelColor(getPixelN(x,y), color);

  // cycles don't start in exactly the same place; "thereabouts"
  cycles[availableCycle].x = constrain((int)x + random(-5,6), 0, RIM_X);
  cycles[availableCycle].y = constrain((int)y + random(-1,2), 0, RIM_Y);
  cycles[availableCycle].color = color;
  cycles[availableCycle].live = true;

  static byte movePref = 0;
  (++movePref) %= 2;
  cycles[availableCycle].movePref = movePref; // likes to go CW or CCW, generally

}

// pass matrix in here
void fadeCycles(Adafruit_NeoPixel &strip) {
  const int fadeAmount = 255 / (CYCLE_TRAIL_LENGTH);

  for( int p=0; p<strip.numPixels(); p++ ) {
    uint32_t c = strip.getPixelColor(p);

    // assuming RGB order.
    byte red = constrain( (int)((c << 8) >> 24) - fadeAmount, 0, 255);
    byte green = constrain( (int)((c << 16) >> 24) - fadeAmount, 0, 255);
    byte blue = constrain( (int)((c << 24) >> 24) - fadeAmount, 0, 255);

    strip.setPixelColor(p, red, green, blue);
  }
}

void moveCycles(Adafruit_NeoPixel &strip, TronCycles *cycles) {
  for( byte c=0; c<MAX_CYCLES; c++ ) {
    if( cycles[c].live ) {
      moveThisCycle(strip, cycles, c);
     }
  }
}

void moveThisCycle(Adafruit_NeoPixel &strip, TronCycles *cycles, byte c) {
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

  const uint32_t Dead = strip.Color(LED_OFF, LED_OFF, LED_OFF);
  const uint32_t White = strip.Color(RED_MAX, GRN_MAX, BLU_MAX);
  // try some moves
  for( byte m=0; m<4; m++ ) {
    if( strip.getPixelColor(getPixelN(moveX[movePref[m]], moveY[movePref[m]])) == Dead ) {
      // good.
      cycles[c].x = moveX[movePref[m]];
      cycles[c].y = moveY[movePref[m]];

      strip.setPixelColor(getPixelN(cycles[c].x,cycles[c].y), cycles[c].color);

      return;
    }
  }
  // uh oh. smash!
  cycles[c].live = false;
  strip.setPixelColor(getPixelN(cycles[c].x,cycles[c].y), White);
}

// returns the i-th pixel mapped to x,y
uint32_t getPixelN(uint32_t x, uint32_t y) {
  if( x >= RIM_X || y >= RIM_Y ) {
    Serial << F("getPixelN error: out of range.  x=") << x << F(" y=") << y << endl << F("Halting.") << endl;
    while(1);
  }
  return( y*RIM_X + x ); // this could be right?
}

/*
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
*/
