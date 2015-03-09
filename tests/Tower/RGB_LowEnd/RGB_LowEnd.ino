#include <Streaming.h>
#include <Metro.h>

// pin locations for outputs
#define LED_R 3 // the PWM pin which drives the red LED
#define LED_G 5 // the PWM pin which drives the green LED
#define LED_B 6 // the PWM pin which drives the blue LED

// timer to switch between modes
#define TESTTIME 5000UL
Metro switchTimer(TESTTIME);

void setup()
{
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(A3)); // or some other unconected pin

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  
  // adjust pins Pins 11 and 3: controlled by timer 2 in phase-correct PWM mode (cycle length = 510)
//  TCCR2B = TCCR2B & 0b11111000 | 0x01; // 980 Hz

  // see: http://jeelabs.org/2011/11/09/fixing-the-arduinos-pwm-2/
  bitSet(TCCR1B, WGM12); // puts Timer1 in Fast PWM mode to match Timer0.

//  analogWrite(LED_R, 255); // red min
  analogWrite(LED_G, 255); // grn min
  analogWrite(LED_B, 255); // blu min
}

void loop()
{
  while(1);
}


