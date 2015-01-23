// Compile for Uno

#include <Streaming.h>
#include <Metro.h>

// number of LED pins
#define N_LED 4
// PWM pins for LEDS
#define RED_PIN 3
#define GRN_PIN 5
#define BLU_PIN 6
#define WHT_PIN 9
// number of relay pins
#define N_RELAY 2
// digital pins for relays
#define FIRE_PIN 7
#define RSPARE_PIN 8

// what is in a name?
const char *ledName[N_LED]={"RED","GRN","BLU","WHT"};
const char *relayName[N_RELAY]={"FIRE","SPARE"};
// pin locations
const byte ledPin[N_LED]={RED_PIN,GRN_PIN,BLU_PIN,WHT_PIN};
const byte relayPin[N_RELAY]={FIRE_PIN,RSPARE_PIN};
// colors are controlled by a current source system
const byte ledOn=255;
const byte ledOff=0;
// relays are controlled by a current sink system
const byte relayOn=LOW;
const byte relayOff=HIGH;

void setup() {
  Serial.begin(115200);
  
  // setup pins and start in off state
  for(int i=0; i<N_LED; i++) {
    setLED(i, ledOff);
    pinMode(ledPin[i], OUTPUT);
  }
  for(int i=0; i<N_RELAY; i++) {
    setRelay(i, relayOff);
    pinMode(relayPin[i], OUTPUT);
  }
  
  Serial << "Startup complete." << endl;
}

void loop() {

  Serial << "LED on/off tests..." << endl; 
  for(int i=0; i<N_LED; i++) {
    setLED(i, ledOn);
    delay(500);
    setLED(i, ledOff);
  }
  Serial << "Relay on/off tests..." << endl; 
  for(int i=0; i<N_RELAY; i++) {
    setRelay(i, relayOn);
    delay(500);
    setRelay(i, relayOff);
  }
  Serial << "LED PWM tests..." << endl; 
  for(int i=0; i<N_LED; i++) {
    for(int j=0; j<10; j++) {
      setLED(i, random(ledOff,ledOn)+1);
      delay(50);
    }
    setLED(i, ledOff);
  }
  
}

void setLED(byte i, byte val) {
  if( val==0 ) { 
    // special case, "off", using digitalWrite to fully turn off pins 5&6
    digitalWrite(ledPin[i], ledOff);
  } else {
    analogWrite(ledPin[i], val);
  }
  Serial << ledName[i] << ": " << val << endl;
}

void setRelay(byte i, byte val) {
  digitalWrite(relayPin[i], val);
  Serial << relayName[i] << ": " << val << endl;
}


