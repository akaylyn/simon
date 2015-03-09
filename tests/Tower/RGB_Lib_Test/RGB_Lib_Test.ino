#include <RGBlink.h>
#include <Streaming.h>
#include <Metro.h>

// pin locations for outputs
#define LED_R 3 // the PWM pin which drives the red LED
#define LED_G 5 // the PWM pin which drives the green LED
#define LED_B 6 // the PWM pin which drives the blue LED

LED light(LED_R, LED_G, LED_B);

// different lighting modes available.
enum LightModes {SOLID, BLINK, FADE, TEST, FLASH};
LightModes lightMode = TEST;

// HSB model 411: http://www.tomjewett.com/colors/hsb.html

// RGBY HSB already defined.  Add some more:
const HSB cyan = {hstep * 3, 255, 255};
const HSB magenta = {hstep * 5, 255, 255};
const HSB white = {0, 0, 255}; // at 0 saturation, hue is meaningless

// timer to switch between modes
#define TESTTIME 5000UL
Metro switchTimer(TESTTIME);
// colors we want to test
#define NCOLORS 5
const HSB colors[NCOLORS] = {red, green, blue, yellow, white};

void setup()
{
  Serial.begin(115200);

  // see: http://jeelabs.org/2011/11/09/fixing-the-arduinos-pwm-2/
  bitSet(TCCR1B, WGM12); // puts Timer1 in Fast PWM mode to match Timer0.

  // random seed.
  randomSeed(analogRead(A3)); // or some other unconected pin

  // start with a HSV test
  light.setMode(lightMode);
}

void loop()
{
  // call the leds update cycle for non-blocking fading
  light.update();

  // cycle through testing colors
  if ( switchTimer.check() ) {

    // set update intervals randomly
//    unsigned long onTime = random(TESTTIME / 4, (TESTTIME * 3) / 4);
//    unsigned long offTime = TESTTIME - onTime;
    unsigned long onTime = TESTTIME/2;
    unsigned long offTime = TESTTIME/2;

    // track the color we're on
    static int colorInd = NCOLORS;

/*
    // determine if we going to SOLID or FADE
    if ( lightMode == SOLID ) {
      
      // keep the same color, just test blinking.
      lightMode = BLINK;
      Serial << "Blinking " << colorInd << " onTime=" << onTime << " offTime=" << offTime << endl;
      
    } else if ( lightMode == BLINK ) {

      // keep the same color, just test fading.
      lightMode = FADE;
      Serial << "Fading " << colorInd << " onTime=" << onTime << " offTime=" << offTime << endl;

    } else {

      // new color, back to solid.
      lightMode = SOLID;
      // select the next color
      colorInd = (colorInd + 1) % NCOLORS; // wrap
      Serial << "Solid " << colorInd << endl;

    }
*/
    colorInd = (colorInd + 1) % NCOLORS; // wrap
    lightMode = FADE;
    
    // update the lighting mode
    light.setMode(lightMode);

    // update intervals; N/A for SOLID but whatever.
    light.setBlink(onTime, offTime);

    // update the color
    light.setColor(colors[colorInd]);

    // reset for next loop
    switchTimer.reset();
  }

}


