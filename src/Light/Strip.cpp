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
Adafruit_NeoMatrix rimJob = Adafruit_NeoMatrix(
    107, 1, 1, 3, RIM_PIN,
    NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT +
    NEO_MATRIX_ROWS +
    NEO_MATRIX_PROGRESSIVE +
    NEO_TILE_BOTTOM + NEO_TILE_LEFT +
    NEO_TILE_ROWS +
    NEO_TILE_PROGRESSIVE,
    NEO_GRB + NEO_KHZ800
    );

// I don't know if this is valid; RIM_PIN is already assigned for the actual matrix above
//Adafruit_NeoPixel rimJobStrip = Adafruit_NeoPixel(RIM_X*RIM_Y, RIM_PIN, NEO_GRB + NEO_KHZ800);

// strips around the buttons
Adafruit_NeoPixel redL = Adafruit_NeoPixel(BUTTON_N, RED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel grnL = Adafruit_NeoPixel(BUTTON_N, GRN_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bluL = Adafruit_NeoPixel(BUTTON_N, BLU_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel yelL = Adafruit_NeoPixel(BUTTON_N, YEL_PIN, NEO_GRB + NEO_KHZ800);

// strip around the middle chotskies
Adafruit_NeoPixel cirL = Adafruit_NeoPixel(CIRCLE_N, CIRCLE_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel placL = Adafruit_NeoPixel(PLACARD_N, PLACARD_PIN, NEO_GRB + NEO_KHZ800);

// define some colors
const uint32_t SweetLoveMakin = rimJob.Color(RED_MAX / 4, GRN_MAX / 6, BLU_MAX / 9);
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

Metro stripUpdateInterval(STRIP_UPDATE);
Metro fasterStripUpdateInterval(STRIP_UPDATE);

// Animations
ConcurrentAnimator animator;
AnimationConfig rimConfig;
AnimationConfig redButtonConfig;
AnimationConfig greenButtonConfig;
AnimationConfig blueButtonConfig;
AnimationConfig yellowButtonConfig;
AnimationConfig circleConfig;
AnimationConfig placardConfig;
AnimationConfig rimConfigStrip;

// Colors
RgbColor red;
RgbColor green;
RgbColor blue;
RgbColor yellow;

// Animation position configurations
LaserWipePosition redLaserPos;
LaserWipePosition greenLaserPos;
LaserWipePosition blueLaserPos;
LaserWipePosition yellowLaserPos;
int rimPos = 0;
int placPos = 0;
int circPos = 0;
ProxPulsePosition proxPulsePos;
ProxPulsePosition idlePos;
GameplayPosition gameplayPos;
GameplayPosition gameplayDecayPos;
TronCycles tronCycles[MAX_CYCLES];
TronPosition tronPosition;

void configureAnimations() {

  // Colors
  red.red = RED_MAX;
  red.green = LED_OFF;
  red.blue = LED_OFF;

  yellow.red = RED_MAX;
  yellow.green = GRN_MAX;
  yellow.blue = LED_OFF;

  green.red = LED_OFF;
  green.green = GRN_MAX;
  green.blue = LED_OFF;

  blue.red = LED_OFF;
  blue.green = LED_OFF;
  blue.blue = BLU_MAX;

  // Neopixel strips
  rimJob.begin();

  rimConfig.name = "Outer rim";
  rimConfig.matrix = &rimJob;
  rimConfig.strip = &rimJob;
  rimConfig.color = blue;
  rimConfig.ready = true;
  rimConfig.position = &proxPulsePos;
  rimConfig.timer = Metro(30UL);
  gameplayPos.decayPos = &gameplayDecayPos;

  // Rim as a strip - TronCycles
  rimConfigStrip.name = "Outer rim - strip";
  rimConfigStrip.strip = &rimJob;
  rimConfigStrip.color = blue;
  rimConfigStrip.ready = true;
  tronPosition.cycles = tronCycles;
  rimConfigStrip.position = &tronPosition;
  rimConfigStrip.timer = Metro(30UL);

  // Init neo pixel strips for the buttons
  redL.begin();
  grnL.begin();
  bluL.begin();
  yelL.begin();

  cirL.begin();
  placL.begin();

  // Red Button
  redButtonConfig.name = "red button";
  redButtonConfig.strip = &redL;
  redButtonConfig.color = red;
  redButtonConfig.ready = true;
  redButtonConfig.position = &redLaserPos;
  redButtonConfig.timer = Metro(50);

  // Green button
  memcpy(&greenButtonConfig, &redButtonConfig, sizeof(AnimationConfig));
  greenButtonConfig.name = "green button";
  greenButtonConfig.strip = &grnL;
  greenButtonConfig.color = green;
  greenButtonConfig.position = &greenLaserPos;

  // Blue button
  memcpy(&blueButtonConfig, &redButtonConfig, sizeof(AnimationConfig));
  blueButtonConfig.name = "blue button";
  blueButtonConfig.strip = &bluL;
  blueButtonConfig.color = blue;
  blueButtonConfig.position = &blueLaserPos;

  // Yellow button
  memcpy(&yellowButtonConfig, &redButtonConfig, sizeof(AnimationConfig));
  yellowButtonConfig.name = "yellow button";
  yellowButtonConfig.strip = &yelL;
  yellowButtonConfig.color = yellow;
  yellowButtonConfig.position = &yellowLaserPos;

  // Circle
  circleConfig.name = "circle";
  circleConfig.strip = &cirL;
  circleConfig.color = yellow;
  circleConfig.ready = true;
  circleConfig.position = &circPos;
  circleConfig.timer = Metro(100);

  // Placard
  placardConfig.name = "placard";
  placardConfig.strip = &placL;
  placardConfig.color = green;
  placardConfig.ready = true;
  placardConfig.position = &placPos;
  placardConfig.timer = Metro(1000);

  clearAllStrips();
}

void mapToAnimation(ConcurrentAnimator animator, systemState state) {
  if (state.animation == A_LaserWipe) {
    animator.animate(laserWipe, redButtonConfig);
    animator.animate(laserWipe, greenButtonConfig);
    animator.animate(laserWipe, blueButtonConfig);
    animator.animate(laserWipe, yellowButtonConfig);
  }

  if (state.animation == A_Idle) {
    rimConfig.position = &idlePos;
    animator.animate(twinkleRand, redButtonConfig);
    animator.animate(twinkleRand, greenButtonConfig);
    animator.animate(twinkleRand, blueButtonConfig);
    animator.animate(twinkleRand, yellowButtonConfig);
    rimConfig.timer.interval(50UL);
    animator.animate(idleMatrix, rimConfig);
    animator.animate(rainbowGlow, placardConfig);
    animator.animate(rainbowGlow, circleConfig);
  }

  if (state.animation == A_ColorWipe) {
    animator.animate(colorWipe, placardConfig);
    animator.animate(colorWipe, circleConfig);
  }

  if (state.animation == A_ProximityPulseMatrix) {
    rimConfig.position = &proxPulsePos;
    rimConfig.color.red = state.light[0].red;
    rimConfig.color.green = state.light[1].green;
    rimConfig.color.blue = state.light[2].blue;
    proxPulsePos.magnitude = state.light[3].red;
    rimConfig.timer.interval(proxPulsePos.magnitude);

    animator.animate(proximityPulseMatrix, rimConfig);
    RgbColor inverse;
    inverse.red = rimConfig.color.green;
    inverse.green = rimConfig.color.blue;
    inverse.blue = rimConfig.color.red;

    circleConfig.color = inverse;
    placardConfig.color = inverse;
    animator.animate(colorWipe, placardConfig);
    animator.animate(colorWipe, circleConfig);
  }

  if (state.animation == A_Gameplay) {
    redL.setBrightness(40);
    grnL.setBrightness(40);
    bluL.setBrightness(40);
    yelL.setBrightness(40);

    setStripColor(redL, BTN_COLOR_RED);
    setStripColor(grnL, BTN_COLOR_GREEN);
    setStripColor(bluL, BTN_COLOR_BLUE);
    setStripColor(yelL, BTN_COLOR_YELLOW);
  }

  if (state.animation == A_GameplayPressed) {
    rimConfig.position = &gameplayPos;
    rimConfig.timer.interval(20UL);
    rimConfig.color.red = state.light[0].red;
    rimConfig.color.green = state.light[1].green;
    rimConfig.color.blue = state.light[2].blue;
    gameplayPos.yellow = state.light[3].red;

    animator.animate(gameplayMatrix, rimConfig);
  }
  if (state.animation == A_GameplayDecay) {
    rimConfig.position = &gameplayDecayPos;
    rimConfig.timer.interval(20UL);
    animator.animate(gameplayDecayMatrix, rimConfig);
  }

  if (state.animation == A_TronCycles) {

    tronPosition.addCycle = false;
    animator.animate(tronLightCycles, rimConfigStrip);

    tronPosition.y = ALL_Y;
    tronPosition.addCycle = true;
    for (int i = 0; i < inst.light[I_RED].red / 64; i++) {
      tronPosition.x = RED_X;
      rimConfigStrip.color = red;
      animator.animate(tronLightCycles, rimConfigStrip);
    }
    for (int i = 0; i < inst.light[I_GRN].green / 64; i++) {
      tronPosition.x = GRN_X;
      rimConfigStrip.color = green;
      animator.animate(tronLightCycles, rimConfigStrip);
    }
    for (int i = 0; i < inst.light[I_BLU].blue / 64; i++) {
      tronPosition.x = BLU_X;
      rimConfigStrip.color = blue;
      animator.animate(tronLightCycles, rimConfigStrip);
    }
    if(inst.light[I_YEL].red > 64 && inst.light[I_YEL].green > 64) {
      for (int i = 0; i < (int)(inst.light[I_YEL].red+inst.light[I_YEL].green) / 128; i++ ) {
        tronPosition.x = YEL_X;
        rimConfigStrip.color = yellow;
        animator.animate(tronLightCycles, rimConfigStrip);
      }
    }
  }

  if (state.animation == A_Clear) {
    clearAllStrips();
  }

  if (state.animation == A_NoRim) {
    setStripColor(rimJob, LED_OFF, LED_OFF, LED_OFF);
  }

}

void clearAllStrips() {
  // Clear all strips
  setStripColor(redL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(grnL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(bluL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(yelL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(placL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(cirL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(rimJob, LED_OFF, LED_OFF, LED_OFF);
}

