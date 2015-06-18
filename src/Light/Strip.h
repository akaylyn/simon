#ifndef Strip_h
#define Strip_h

#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <Streaming.h>
#include <Metro.h>
#include <EasyTransfer.h>
#include <LightMessage.h> // common message definition

// watchdog timer
#include <avr/wdt.h>

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
#define RED_PIN 4 // wireito button DI pin.  Include a 330 Ohm resistor in series.
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
// communications with Console module via Serial port
#define LightComms Serial1
#define LIGHT_COMMS_RATE 19200
// LED brightness is not equivalent across colors.  Higher wavelengths are dimmed to balance.
#define RED_MAX 255
#define GRN_MAX 220
#define BLU_MAX 210

#define LED_OFF 0 // makes it a litle brighter in the Console.  0 is fine, too.
// reduce intensity at each update, so this is the amount of time we can expect a pixel to last
#define PIXEL_TTL 3000UL

// since the pixels have a TTL, we need to add some when there's nothing going on.
#define STRIP_ADD_PIXEL PIXEL_TTL/3

// update the automata on the rim at this interval
#define STRIP_UPDATE 100
#define FASTER_STIRP_UPDATE 30

// count memory usage for LEDs, which is reported at startup.
#define TOTAL_LED_MEM (RIM_N + BUTTON_N*4 + MIDDLE_N)*3
#endif

