#include "Animations.h"

void setStripColor(Adafruit_NeoPixel &strip, int r, int g, int b) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
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
    setStripColor(strip, LED_OFF, LED_OFF, LED_OFF);
  } else {
    ++next;
  }

  strip.setPixelColor(next, strip.Color(r, g, b));
  Serial << F("Set pixel: ") << next << endl;
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
  /*int* pos = (int*) posData;
  int magnitude = (*pos);
  */

  int point = 0;

  Serial << r << " " << g << " " << b << endl;
  //(*pos) = next;
  for (int i = magnitude; i < matrix.width(); i++) {

    if (r < 85) {
      matrix.drawPixel(point-magnitude, 1, matrix.Color(r,g,b));
      matrix.drawPixel(point+magnitude, 1, matrix.Color(r,g,b));
    }

    if (85 < r < 170) {
      matrix.drawPixel(i, 1, matrix.Color(r,g,b));
    }

    if (r > 170) {
      matrix.drawPixel(i, 0, matrix.Color(r,g,b));
    }
  }
  //magnitude = magnitude++;
  //(*pos) = magnitude;
}

