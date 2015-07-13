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
  // set background
  //	 stripSet(bg,0);
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.setPixelColor(random(strip.numPixels()), random(255));
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
