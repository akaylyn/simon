#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include "Light.h"
#include "ButtonTest.h"

// TODO: move to NeoMatrix to take advantage of the 3x stacked strips
#include <Streaming.h>
#include <Metro.h>
#include <EasyTransfer.h>
#include <LightMessage.h> // common message definition

// watchdog timer
#include <avr/wdt.h>


#define PIN 4

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(49, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix strip = rimJob;

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
    Serial.begin(115200);
    rimJob.begin();
    rimJob.show();
    //rimJob.setTextWrap(true);
    //rimJob.Color(255, 0, 0);
    //strip.begin();
    //strip.show(); // Initialize all pixels to 'off'
    redL.begin();
    redL.show();
    /*grnL.begin();
    grnL.show();
    bluL.begin();
    bluL.show();
    yelL.begin();
    yelL.show();
    */

    //setStripColor(redL, RED_MAX, GRN_MAX, BLU_MAX);
    //setStripColor(rimJob, RED_MAX, GRN_MAX, BLU_MAX);
}

void setStripColor(Adafruit_NeoPixel &strip, int r, int g, int b) {
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}

static uint8_t lastPosition = 0;
static uint8_t buttonLastPosition = 0;
static bool rimReady = true;
static bool btnReady = true;
void loop() {
    if (stripUpdateInterval.check()) {
        Serial << "------------------------ start loop" << endl;

    /*
        setStripColor(redL, RED_MAX, LED_OFF, LED_OFF);
        colorWipe(redL, redL.Color(127, 127, 0), 20);
        */
    /*    colorWipe(grnL, Blu, 20);
        colorWipe(grnL, Yel, 20);
        colorWipe(yelL, Red, 20);
        colorWipe(yelL, Grn, 20);
        colorWipe(bluL, Blu, 20);
        colorWipe(bluL, Yel, 20);
        */
        // Some example procedures showing how to display to the pixels:
        /*colorWipeMatrix(rimJob, Grn, 1); // Green
        colorWipeMatrix(rimJob, Yel, 1); // Blue
        colorWipeMatrix(rimJob, Red, 1); // Red
        colorWipeMatrix(rimJob, Blu, 1); // Blue
        */

        // Send a theater pixel chase in...
        /*
        theaterChase(strip.Color(127, 127, 127), 50); // White
        theaterChase(strip.Color(127,   0,   0), 50); // Red
        theaterChase(strip.Color(  0,   0, 127), 50); // Blue

        rainbow(20);
        rainbowCycle(20);
        theaterChaseRainbow(50);
        */

        void (*wipeStrip)(Adafruit_NeoPixel&, int, int, int, uint8_t) = colorWipe;

        // rim
        calculateAnimation(wipeStrip, rimJob, RED_MAX, GRN_MAX, LED_OFF, lastPosition, rimReady);
        push(strip, rimReady);
        lastPosition++;
        if (lastPosition > rimJob.numPixels()) lastPosition = 0;

        // red button
        calculateAnimation(wipeStrip, redL, LED_OFF, GRN_MAX, LED_OFF, buttonLastPosition, btnReady);
        push(redL, btnReady);

        buttonLastPosition++;
        if (buttonLastPosition > redL.numPixels()) buttonLastPosition = 0;

        stripUpdateInterval.reset();
    }
}

void push(Adafruit_NeoPixel &strip, bool &ready) {
    static Metro timer(1);
    if (!timer.check()) return;
    strip.show();
    ready = true;
}

void calculateAnimation(void (*animate)(Adafruit_NeoPixel&, int, int, int, uint8_t),
        Adafruit_NeoPixel &strip, int r, int g, int b, uint8_t wait, bool &ready) {
    if (!ready) {
        Serial << "calcAnimation !! Not Ready !! " << strip.numPixels() << endl;
        return;
    }
    Serial << "calcAnimation" << endl;
    (*animate)(strip, r, g, b, wait);
    ready = false;
}

void colorWipeMatrix(Adafruit_NeoMatrix &matrix, uint32_t c, uint8_t wait) {
    for (uint16_t x = 0; x < matrix.width(); x++) {
        matrix.drawPixel(x, 0, c);
        matrix.drawPixel(x, 1, c);
        matrix.drawPixel(x, 2, c);
        matrix.show();
    }
}

// Fill the dots one after the other with a color
void colorWipe(Adafruit_NeoPixel &button, int r, int g, int b, uint8_t wait) {
    button.setPixelColor(wait, button.Color(r, g, b));
    Serial << F("Set pixel: ") << wait << endl;
}

// color wipes the last 8 pixels
void laserWipe(Adafruit_NeoPixel &button, int r, int g, int b, uint8_t wait, int times = 3) {
    int currTimes = 0;

    while (currTimes < times) {
        for (uint16_t i = 0; i < button.numPixels(); i++) {
            button.setPixelColor(i, button.Color(r, g, b));
            button.show();
            delay(wait);
        }
        times++;
    }
}

void rainbow(uint8_t wait) {
    uint16_t i, j;

    for(j=0; j<256; j++) {
        for(i=0; i<strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel((i+j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
    uint16_t i, j;

    for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
        for(i=0; i< strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
    for (int j=0; j<10; j++) {  //do 10 cycles of chasing
        for (int q=0; q < 3; q++) {
            for (int i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, c);    //turn every third pixel on
            }
            strip.show();

            delay(wait);

            for (int i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, 0);        //turn every third pixel off
            }
        }
    }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
    for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
        for (int q=0; q < 3; q++) {
            for (int i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
            }
            strip.show();

            delay(wait);

            for (int i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, 0);        //turn every third pixel off
            }
        }
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}

