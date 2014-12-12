#include "Light.h"

// configures buttons at startup
void configureLights() {
  digitalWrite(PIXELS_RED, HIGH);
  pinMode(PIXELS_RED, OUTPUT);
  digitalWrite(PIXELS_YEL, HIGH);
  pinMode(PIXELS_YEL, OUTPUT);
  digitalWrite(PIXELS_BLU, HIGH);
  pinMode(PIXELS_BLU, OUTPUT);
  digitalWrite(PIXELS_GRN, HIGH);
  pinMode(PIXELS_GRN, OUTPUT);
}

