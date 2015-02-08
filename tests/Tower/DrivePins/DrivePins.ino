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

#define SYSTEM_ENABLE A0
#define GAMEPLAY_ENABLE A1

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
  pinMode(SYSTEM_ENABLE, INPUT_PULLUP);
  pinMode(GAMEPLAY_ENABLE, INPUT_PULLUP);  

  Serial << "Startup complete." << endl;
}

void loop() {

  Serial << "System Enable: " << digitalRead(SYSTEM_ENABLE) << endl;
  Serial << "Gameplay Enable: " << digitalRead(GAMEPLAY_ENABLE) << endl;
 
  Serial << "LED on/off tests..." << endl; 
  for(int i=0; i<N_LED; i++) {
    setLED(i, ledOn);
    delay(500);
    setLED(i, ledOff);
  }
  if( digitalRead(SYSTEM_ENABLE) == LOW) {
    Serial << "Relay on/off tests..." << endl; 
    for(int i=0; i<N_RELAY; i++) {
      setRelay(i, relayOn);
      delay(500);
      setRelay(i, relayOff);
    }
  }
  
  if( digitalRead(GAMEPLAY_ENABLE) == LOW) {
    Serial << "LED PWM tests..." << endl; 
    for(int i=0; i<N_LED; i++) {
      for(int j=ledOff; j<ledOn; j++) {
        setLED(i,j);
        delay(5);
      }
      setLED(i, ledOff);
    }
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


