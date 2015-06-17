#ifndef Light_h
#define Light_h
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <Streaming.h>
#include <Metro.h>
#include <EasyTransfer.h>
//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

#include "ConcurrentAnimator.h"

// RIM of LEDs
#define RIM_PIN 3 // wire to rim DI pin.  Include a 330 Ohm resistor in series.
// geometry
#define RIM_N 108 // 108*3 best if divisible by 4
#define RIM_SEG_LENGTH 27 // floor(RIM_N/4)=27
#define YEL_SEG_START 46 // start yellow at this pixel
#define GRN_SEG_START YEL_SEG_START+RIM_SEG_LENGTH
#define RED_SEG_START GRN_SEG_START+RIM_SEG_LENGTH
#define BLU_SEG_START RED_SEG_START+RIM_SEG_LENGTH

// 4x touch lighting strips
#define RED_PIN 4 // wire to button DI pin.  Include a 330 Ohm resistor in series.
#define GRN_PIN 5 // wire to button DI pin.  Include a 330 Ohm resistor in series.
#define BLU_PIN 6 // wire to button DI pin.  Include a 330 Ohm resistor in series.
#define YEL_PIN 7 // wire to button DI pin.  Include a 330 Ohm resistor in series.
// geometry
#define BUTTON_N 49 // wrapped around each button

// 2x chotsky lighting
#define CIRCLE_PIN 8 //
// geometry
#define CIRCLE_N 18 // wrapped around middle chotsky
//
#define PLACARD_PIN 8 //
// geometry
#define PLACARD_N 18 // wrapped around middle chotsky

// LED indicator to ack button presses
#define LED_PIN 13

// button pins.  wire to Mega GPIO, bring LOW to indicate pressed.
//create object
EasyTransfer ET;

//give a name to the group of data
systemState inst;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

// strip around the inner rim
//Adafruit_NeoPixel rimJob = Adafruit_NeoPixel(RIM_N, RIM_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix rimJob = Adafruit_NeoMatrix(
        108, 1, 1, 3, RIM_PIN,
        NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT +
        NEO_MATRIX_ROWS +
        NEO_MATRIX_PROGRESSIVE +
        NEO_TILE_BOTTOM + NEO_TILE_LEFT +
        NEO_TILE_ROWS +
        NEO_TILE_PROGRESSIVE,
        NEO_GRB + NEO_KHZ800
        );

// strips around the buttons
Adafruit_NeoPixel redL = Adafruit_NeoPixel(BUTTON_N, RED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel grnL = Adafruit_NeoPixel(BUTTON_N, GRN_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bluL = Adafruit_NeoPixel(BUTTON_N, BLU_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel yelL = Adafruit_NeoPixel(BUTTON_N, YEL_PIN, NEO_GRB + NEO_KHZ800);

// strip around the middle chotskies
Adafruit_NeoPixel cirL = Adafruit_NeoPixel(CIRCLE_N, PLACARD_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel placL = Adafruit_NeoPixel(PLACARD_N, PLACARD_PIN, NEO_GRB + NEO_KHZ800);

// count memory usage for LEDs, which is reported at startup.
#define TOTAL_LED_MEM (RIM_N + BUTTON_N*4 + CIRCLE_N + PLACARD_N)*3

// using these maxima, we get a balanced white
#define RED_MAX 255
#define BLU_MAX 220
#define GRN_MAX 210

// pixels should live this long 
#define PIXEL_TTL 3000UL
#define STRIP_UPDATE 20UL

// would keep LEDs from going completely off if >0
#define LED_OFF 0


#endif

