#include "Light.h"

// system outputs
// lights
LED redLight(LED_RED);
LED grnLight(LED_GRN);
LED bluLight(LED_BLU);
LED yelLight(LED_YEL);

// configures lights at startup
void lightStart() {
  Serial << F("Light: startup.") << endl;
  
  ledStart();
  pixelsStart();
}

// configures lights for buttons at startup
void ledStart() {
  ledSet(I_ALL, LED_OFF);
}

// configures lights on rim and buttons at startup
void pixelsStart() {
  // set the value HIGH first, then configure OUTPUT
  pixelSet(I_ALL, PIXELS_OFF);
  
  pinMode(PIXELS_RED, OUTPUT);
  pinMode(PIXELS_GRN, OUTPUT);
  pinMode(PIXELS_BLU, OUTPUT);
  pinMode(PIXELS_YEL, OUTPUT);
}

// set pixel and LED (or I_ALL)  to a value
void lightSet(byte lightIndex, byte ledValue) {
  // set the LEDs
  ledSet(lightIndex, ledValue);
  
  // set the Pixels by mapping [0,255] -> [0,1] by PIXEL_THRESHOLD
  if( ledValue >= PIXEL_THRESHOLD ) pixelSet(lightIndex, PIXELS_ON);
  else pixelSet(lightIndex, PIXELS_OFF);
}

// set led light (or I_ALL) to a value
void ledSet(byte lightIndex, byte ledValue) {
  switch ( lightIndex ) {
    case I_RED:
      redLight.setValue(ledValue);
      break;
    case I_GRN:
      grnLight.setValue(ledValue);
      break;
    case I_BLU:
      bluLight.setValue(ledValue);
      break;
    case I_YEL:
      yelLight.setValue(ledValue);
      break;
    case I_ALL:
      redLight.setValue(ledValue);
      grnLight.setValue(ledValue);
      bluLight.setValue(ledValue);
      yelLight.setValue(ledValue);
      break;
  }
}

// set pixel light (or I_ALL) to a value
void pixelSet(byte lightIndex, byte pixelValue) {
  switch ( lightIndex ) {
    case I_RED:
      digitalWrite(PIXELS_RED, pixelValue);
      break;
    case I_GRN:
      digitalWrite(PIXELS_GRN, pixelValue);
      break;
    case I_BLU:
      digitalWrite(PIXELS_BLU, pixelValue);
      break;
    case I_YEL:
      digitalWrite(PIXELS_YEL, pixelValue);
      break;
    case I_ALL:
      digitalWrite(PIXELS_RED, pixelValue);
      digitalWrite(PIXELS_GRN, pixelValue);
      digitalWrite(PIXELS_BLU, pixelValue);
      digitalWrite(PIXELS_YEL, pixelValue);
      break;
  }
}


