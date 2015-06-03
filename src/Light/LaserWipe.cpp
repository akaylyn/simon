#include "LaserWipe.h"

extern Animations animations;

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
animations.Btn.LaserWipe = laserWipe;

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

