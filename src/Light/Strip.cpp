#include "Strip.h"

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

// define some colors
const uint32_t SweetLoveMakin = rimJob.Color(RED_MAX / 4, GRN_MAX / 6, BLU_MAX / 9);
const uint32_t Red = rimJob.Color(RED_MAX, LED_OFF, LED_OFF);
const uint32_t Yel = rimJob.Color(RED_MAX, GRN_MAX, LED_OFF);
const uint32_t Grn = rimJob.Color(LED_OFF, GRN_MAX, LED_OFF);
const uint32_t Blu = rimJob.Color(LED_OFF, LED_OFF, BLU_MAX);
const uint32_t Dead = rimJob.Color(LED_OFF, LED_OFF, LED_OFF);
const uint32_t BTN_COLOR_RED = redL.Color(RED_MAX, LED_OFF, LED_OFF);
const uint32_t BTN_COLOR_YELLOW = redL.Color(RED_MAX, GRN_MAX, LED_OFF);
const uint32_t BTN_COLOR_GREEN = redL.Color(LED_OFF, GRN_MAX, LED_OFF);
const uint32_t BTN_COLOR_BLUE = redL.Color(LED_OFF, LED_OFF, BLU_MAX);
// track when we need to send update.
boolean rimUpdated = false;
boolean redUpdated = false;
boolean grnUpdated = false;
boolean bluUpdated = false;
boolean yelUpdated = false;
boolean midUpdated = false;

Metro quietUpdateInterval(STRIP_ADD_PIXEL);
Metro stripUpdateInterval(STRIP_UPDATE);
Metro fasterStripUpdateInterval(STRIP_UPDATE);

#endif

