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

class Fx {
  public:
    void begin();
    void reset(); // reset the board
    
    void on(Trigger); // will never stop
    
    void off(Trigger);
    void allOff();
  private:
};

Fx fx;

void Fx::begin() {
  Serial << F("Fx: setup.") << endl;
  
  // setup pins
  for(int i=0; i<N_TRIGGER; i++) {
    digitalWrite(pin[i], HIGH);
    pinMode(pin[i],OUTPUT);
  }
  
  reset();  
    
  Serial << F("Fx: setup complete.") << endl;
}

void Fx::reset() {
  // reset the board
  pinMode(FX_RESET, OUTPUT);
  digitalWrite(FX_RESET, LOW);
  delay(10);
  digitalWrite(FX_RESET, HIGH);
}

void Fx::on(Trigger t) {
  digitalWrite(pin[t], LOW);
}
void Fx::off(Trigger t) {
  digitalWrite(pin[t], HIGH);
}

void Fx::allOff() {
  // all off
  for(int i=0; i<N_TRIGGER; i++) {
    digitalWrite(pin[i], HIGH);
  }
}

void setup() {
  Serial.begin(115200);
  fx.begin();
  
  Serial << "BAFF's..." << endl;
  fx.on(BAFF);
  delay(10000);
  fx.off(BAFF);  
  
  Serial << "RED's..." << endl;
  fx.on(RED); delay(1000); fx.off(RED); delay(1000);
  fx.on(GRN); delay(1000); fx.off(GRN); delay(1000);
  fx.on(BLU); delay(1000); fx.off(BLU); delay(1000);
  fx.on(YEL); delay(1000); fx.off(YEL); delay(1000);
  fx.on(WRONG); delay(1000); fx.off(WRONG); delay(1000);

}

void loop() {
}
