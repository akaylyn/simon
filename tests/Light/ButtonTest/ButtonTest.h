#ifndef ButtonTest_h
#define ButtonTest_h

void calculateAnimation(
    void (*animate)(Adafruit_NeoPixel&, int, int, int, uint8_t),
    Adafruit_NeoPixel &strip, int r, int g, int b, uint8_t wait, bool &ready
    );
#endif
