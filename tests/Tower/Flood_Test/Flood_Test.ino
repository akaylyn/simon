#include <Streaming.h>
#include <Metro.h>

#define RED 2
#define GRN 3
#define BLU 4
#define WHT 5

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial << "Setup." << endl;
  
  pinMode(RED, OUTPUT);
  set(RED, 0);
  pinMode(GRN, OUTPUT);
  set(GRN, 0);
  pinMode(BLU, OUTPUT);
  set(BLU, 0);
  pinMode(WHT, OUTPUT);
  set(WHT, 0);
  
  Serial << "Setup complete." << endl;
}

void loop() {

/*  
  set(RED,255);
  delay(5000);
  set(RED,0);
*/
  set(GRN,20);
  delay(5000);
  set(GRN,0);
  delay(5000);
/*
  set(BLU,255);
  delay(5000);
  set(BLU,0);

  set(WHT,255);
  delay(5000);
  set(WHT,0);
  */
}

void set(int pin, byte level) {
  analogWrite(pin, level);
}
