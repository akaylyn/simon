#include <Streaming.h>

#define B_RED 4
#define B_GRN 5
#define B_BLU 3
#define B_YEL 6
#define B_STK 7
#define J_X A0
#define J_Y A1
#define GROUND 2 // Wire to GND
// add wire from +3.3V to "5V" on shield.


void setup() {
  pinMode(GROUND, OUTPUT);
  digitalWrite(GROUND, LOW); // wire to GN
  
  // buttons
  pinMode(B_RED, INPUT); 
  pinMode(B_GRN, INPUT); 
  pinMode(B_BLU, INPUT); 
  pinMode(B_YEL, INPUT); 
  pinMode(B_STK, INPUT); 
  
  // analog
  pinMode(J_X, INPUT);
  pinMode(J_Y, INPUT);
    
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial << "\tR:" << digitalRead(B_RED); 
  Serial << "\tG:" << digitalRead(B_GRN); 
  Serial << "\tB:" << digitalRead(B_BLU); 
  Serial << "\tY:" << digitalRead(B_YEL); 
  Serial << "\tS:" << digitalRead(B_STK); 
  Serial << "\tX:" << analogRead(J_X); 
  Serial << "\tY:" << analogRead(J_Y); 
  Serial << endl;
  
  delay(100);
}
