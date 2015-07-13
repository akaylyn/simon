#ifndef Strip_h
#define Strip_h

#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <Streaming.h>
#include <Metro.h>
#include <EasyTransfer.h>
#include <Simon_Common.h> // common message definition

#include "AnimationConfig.h"
#include "Animations.h"
#include "ConcurrentAnimator.h"
#include "AnimateFunc.h"

// GRN > RED
//  ^     v
// YEL < BLU

// RIM of LEDs
#define RIM_PIN 3 // wire to rim DI pin.  Include a 330 Ohm resistor in series.
// geometry
#define RIM_X 107
#define RIM_Y 3
#define RED_X 7
#define BLU_X 34 // delta 27
#define YEL_X 61 // delta 27
#define GRN_X 88 // delta 27
#define ALL_Y 1

// 4x touch lighting strips
#define RED_PIN 4 // wireito button DI pin.  Include a 330 Ohm resistor in series.
#define GRN_PIN 5 // wire to button DI pin.  Include a 330 Ohm resistor in series.
#define BLU_PIN 6 // wire to button DI pin.  Include a 330 Ohm resistor in series.
#define YEL_PIN 7 // wire to button DI pin.  Include a 330 Ohm resistor in series.
// geometry
#define BUTTON_N 49 // wrapped around each button

// 2x chotsky lighting
#define CIRCLE_PIN 8 //
// geometry
#define CIRCLE_N 18 // wrapped around coaster chotsky
#define PLACARD_PIN 9 //
// geometry
#define PLACARD_N 18 // wrapped around Simon chotsky

// LED indicator to ack button presses
#define LED_PIN 13

// LED brightness is not equivalent across colors.  Higher wavelengths are dimmed to balance.
#define RED_MAX 255
#define GRN_MAX 255 // 220
#define BLU_MAX 255 // 210
#define LED_OFF 0 // makes it a litle brighter in the Console.  0 is fine, too.

// update the automata on the rim at this interval
#define STRIP_UPDATE 20UL
#define FASTER_STIRP_UPDATE 10UL

// count memory usage for LEDs, which is reported at startup.
#define TOTAL_LED_MEM (RIM_X*RIM_Y + BUTTON_N*4 + CIRCLE_N + PLACARD_N)*3

void configureAnimations();
void mapToAnimation(ConcurrentAnimator, systemState);
void clearAll();

#endif

