#include "ColorWipe.h"
#include "Animations.h"

extern Animations animations;

void colorWipeMatrix(Adafruit_NeoMatrix &matrix, int c, int wait) {
  for (uint16_t x = 0; x < matrix.width(); x++) {
    matrix.drawPixel(x, 0, c);
    matrix.drawPixel(x, 1, c);
    matrix.drawPixel(x, 2, c);
    matrix.show();
  }
}
animations.Test = colorWipeMatrix;

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
  Serial << F("Set pixel: ") << next << endl;
  posData = (void*) next;
}
animations.Btn.ColorWipe = colorWipe;

