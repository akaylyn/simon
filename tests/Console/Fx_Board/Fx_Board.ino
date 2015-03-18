#include <Streaming.h>
#include <Metro.h>

#define N_TRIGGER 11
enum Trigger {
  BAFF=0,
  RED=1, GRN=2, BLU=3, YEL=4, WRONG=5,
  // unused, currently, from here on out
  U1=6, U2=7, U3=8, U4=9, U5=10
};
const int pin[N_TRIGGER] = {
  30,
  31,32,33,34,35,
  36,37,38,39,40
};

#define FX_RESET 41
#define FX_VOL_UP 42
#define FX_VOL_DOWN 43 
#define FX_PRESS_DELAY 1UL

class Fx {
public:
  void begin();
  void reset(); // reset the board

  void fxOn(Trigger t);
  void fxOff(Trigger t);
  void fxAllOff();
  void fxVolUp();
  void fxVolDown();
  void fxVolFull();

private:
};

Fx fx;

void Fx::begin() {
 // Fx board
  Serial << F("Fx: setup.") << endl;
  
  // setup pins
  for(int i=0; i<N_TRIGGER; i++) {
    digitalWrite(pin[i], HIGH);
    pinMode(pin[i],OUTPUT);
  }
  
  // reset the board
  pinMode(FX_RESET, OUTPUT);
  digitalWrite(FX_RESET, LOW);
  delay(FX_PRESS_DELAY);
  digitalWrite(FX_RESET, HIGH);
  delay(100);

  // volume settings
  digitalWrite(FX_VOL_DOWN, HIGH);  
  pinMode(FX_VOL_DOWN, OUTPUT);
  digitalWrite(FX_VOL_UP, HIGH);  
  pinMode(FX_VOL_UP, OUTPUT);
  fxVolFull();
  

  Serial << F("Fx: setup complete.") << endl;
}

void Fx::fxOn(Trigger t) {
  digitalWrite(pin[t], LOW);
//  Serial << "Fx: " << t << " pin: " << pin[t] << endl;
}

void Fx::fxOff(Trigger t) {
  digitalWrite(pin[t], HIGH);
}

void Fx::fxAllOff() {
  // all off
  for(int i=0; i<N_TRIGGER; i++) {
    digitalWrite(pin[i], HIGH);
  }
}

void Fx::fxVolUp() {
  digitalWrite(FX_VOL_UP, LOW);
  delay(FX_PRESS_DELAY);
  digitalWrite(FX_VOL_UP, HIGH);  
  delay(FX_PRESS_DELAY);
}
void Fx::fxVolDown() {
  digitalWrite(FX_VOL_DOWN, LOW);
  delay(FX_PRESS_DELAY);
  digitalWrite(FX_VOL_DOWN, HIGH);  
  delay(FX_PRESS_DELAY);
}
void Fx::fxVolFull() {
  for(int i=0;i<512;i++ ) fxVolUp();  
}


void setup() {
  Serial.begin(115200);
  fx.begin();
/*
  Serial << "BAFF's..." << endl;
  fx.fxOn(BAFF);
  delay(10000);
  fx.fxOff(BAFF);  
*/
  Serial << "RED's..." << endl;

/*
  delay(1000); 
  fx.fxOff(RED); 
  delay(1000);
  fx.fxOn(GRN); 
  delay(1000); 
  fx.fxOff(GRN); 
  delay(1000);
  fx.fxOn(BLU); 
  delay(1000); 
  fx.fxOff(BLU); 
  delay(1000);
  fx.fxOn(YEL); 
  delay(1000); 
  fx.fxOff(YEL); 
  delay(1000);
  fx.fxOn(WRONG); 
  delay(1000); 
  fx.fxOff(WRONG); 
  delay(1000);
*/
}

void loop() {
  fx.fxOn(RED); 
  for(int i=1;i<5000;i++) {
//    Serial << i << endl;
    fx.fxVolDown();
  }
  delay(1000);
  fx.fxVolFull();
  delay(1000);
}


