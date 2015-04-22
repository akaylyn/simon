
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
// TODO: move to NeoMatrix to take advantage of the 3x stacked strips
#include <Streaming.h>
#include <Metro.h>
#include <EasyTransfer.h>
#include <LightMessage.h> // common message definition

// watchdog timer
#include <avr/wdt.h>

// RIM of LEDs
#define RIM_PIN 3 // wire to rim DI pin.  Include a 330 Ohm resistor in series.
// geometry
#define RIM_N 107*3 // best if divisible by 4
#define RIM_SEG_LENGTH 27 // floor(RIM_N/4)=27
#define YEL_SEG_START 12 // start yellow at this pixel
#define BLU_SEG_START YEL_SEG_START+RIM_SEG_LENGTH
#define RED_SEG_START BLU_SEG_START+RIM_SEG_LENGTH
#define GRN_SEG_START RED_SEG_START+RIM_SEG_LENGTH

// 4x touch lighting strips
#define RED_PIN 4 // wire to button DI pin.  Include a 330 Ohm resistor in series.
#define GRN_PIN 5 // wire to button DI pin.  Include a 330 Ohm resistor in series.
#define BLU_PIN 6 // wire to button DI pin.  Include a 330 Ohm resistor in series.
#define YEL_PIN 7 // wire to button DI pin.  Include a 330 Ohm resistor in series.
// geometry
#define BUTTON_N 49 // wrapped around each button

// 1x chotsky lighting
#define MIDDLE_PIN 8 //
// geometry
#define MIDDLE_N 18 // wrapped around middle chotsky

// LED indicator to ack button presses
#define LED_PIN 13

// button pins.  wire to Mega GPIO, bring LOW to indicate pressed.
//create object
EasyTransfer ET;

//give a name to the group of data
LightET lightInst;

// communications with Console module via Serial port
#define LightComms Serial1
#define LIGHT_COMMS_RATE 19200

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

// strip around the middle chotsky
Adafruit_NeoPixel midL = Adafruit_NeoPixel(MIDDLE_N, MIDDLE_PIN, NEO_GRB + NEO_KHZ800);

// LED brightness is not equivalent across colors.  Higher wavelengths are dimmed to balance.
#define RED_MAX 255
#define GRN_MAX 220
#define BLU_MAX 210

#define LED_OFF 0 // makes it a litle brighter in the Console.  0 is fine, too.
// define some colors
const uint32_t SweetLoveMakin = rimJob.Color(RED_MAX / 4, GRN_MAX / 6, BLU_MAX / 9);
const uint32_t Red = rimJob.Color(RED_MAX, LED_OFF, LED_OFF);
const uint32_t Yel = rimJob.Color(RED_MAX, GRN_MAX, LED_OFF);
const uint32_t Grn = rimJob.Color(LED_OFF, GRN_MAX, LED_OFF);
const uint32_t Blu = rimJob.Color(LED_OFF, LED_OFF, BLU_MAX);
const uint32_t Dead = rimJob.Color(LED_OFF, LED_OFF, LED_OFF);

// track when we need to send update.
boolean rimUpdated = false;
boolean redUpdated = false;
boolean grnUpdated = false;
boolean bluUpdated = false;
boolean yelUpdated = false;
boolean midUpdated = false;

// reduce intensity at each update, so this is the amount of time we can expect a pixel to last
#define PIXEL_TTL 3000UL

// since the pixels have a TTL, we need to add some when there's nothing going on.
#define STRIP_ADD_PIXEL PIXEL_TTL/3
Metro quietUpdateInterval(STRIP_ADD_PIXEL);

// update the automata on the rim at this interval
#define STRIP_UPDATE 100UL
Metro stripUpdateInterval(STRIP_UPDATE);

// count memory usage for LEDs, which is reported at startup.
#define TOTAL_LED_MEM (RIM_N + BUTTON_N*4 + MIDDLE_N)*3


