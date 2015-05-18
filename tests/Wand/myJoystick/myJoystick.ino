#include <Streaming.h>

#define B_RED 22 
#define B_GRN 21
#define B_BLU 23
#define B_YEL 20
#define B_STK 19
#define J_X A0 // D24, A0
#define J_Y A1 // D25, A1
// add wire from +3.3V to "5V" on shield.


void setup() {
//  analogReference(DEFAULT);
  
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
