#include <Streaming.h>

#define R_PIN 5
#define G_PIN 6
#define B_PIN 10

#define N_PINS 3
int pin[N_PINS] = {R_PIN, G_PIN, B_PIN};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  for( int i=0; i<N_PINS; i++ ) {
    pinMode(pin[i], OUTPUT);
    digitalWrite(pin[i], HIGH);
  }
}

void loop() {
  for( int i=0; i<N_PINS; i++ ) {
    Serial << i << endl;
    digitalWrite(pin[i], LOW);
    delay(1000);
    digitalWrite(pin[i], HIGH);
    
    for( int j=0; j<=255; j++ ) {
      Serial << i << " set " << j << endl;
      analogWrite(pin[i], 255-j);
      delay(50);
    }
    digitalWrite(pin[i], HIGH);
    
  }
  Serial << "done" << endl;
}
