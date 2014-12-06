#include <Streaming.h>

#define FLAME 4 // JeeNode Port 1, pin 2 (DIO)
#define OTHER A0 // JeeNode Port 1, pin 5 (AIO)
#define GPIO1 7 // JeeNode Port 4, pin 2 (DIO)
#define GPIO2 A3 // JeeNode Port 4, pin 5 (AIO)

void setup() {
  // put your setup code here, to run once:
  for( int i=2;i<20;i++) {
    pinMode(i,OUTPUT);
  }
  
  Serial.begin(115200);  
}

void loop() {
  Serial << endl << "pin: " << FLAME << " on ";
  digitalWrite(FLAME,HIGH);
  delay(1000);
  Serial << " off ";
  digitalWrite(FLAME,LOW);
  delay(1000);

  Serial << endl << "pin: " << OTHER << " on ";
  digitalWrite(OTHER,HIGH);
  delay(1000);
  Serial << " off ";
  digitalWrite(OTHER,LOW);
  delay(1000);
}
