/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 */
 
#include <Streaming.h>
#include <IRremote.h>

unsigned long baseline=0;

IRsend irsend;

// submersibles
uint16_t specAddress = 0x00FF;
byte onCode = 0xB0;
byte offCode = 0xF8;

/*
// 50W floods, strips
uint16_t specAddress = 0x00F7;
byte onCode = 0xC0;
byte offCode = 0x40;
*/
/*
// minisubs
uint16_t specAddress = 0x01FE;
byte onCode = 0x48;
byte offCode = 0x58;
*/
/*
// 10W floods
uint16_t specAddress = 0x00F7;
byte onCode = 0xC0;
byte offCode = 0x40;
*/
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  pinMode(A1, INPUT);
  
  send(specAddress, onCode);
  delay(500);
  send(specAddress, offCode);
  delay(500);
  
  for(int i=0; i<20; i++ ) 
    baseline+=analogRead(A1);
    
  baseline/=20;
  
  Serial << "Baseline = " << baseline << endl;
}

// the loop routine runs over and over again forever:
void loop() {
  
  static uint16_t address = 0xFF00;
  
  send(address, onCode);
  
  if( checkSensor() ) {
    Serial << "Address=" << dec2binWzerofill(address) << "\t" << _HEX(address) << endl;
    send(specAddress, offCode);
  }
  
  address++;
  
//  if( (address % 100) == 0 ) Serial << address << endl;
}

boolean checkSensor() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(A1);
  // print out the value you read:
  if( sensorValue <= (baseline-100) ) return( true );
  return( false );
  
}

void send(uint16_t address, byte code) {
  unsigned long msg = generateCode(address, code);
//  Serial << "Send: " << _HEX(msg);
  
  unsigned long tic = millis();
  irsend.sendNEC(msg, 32);
  unsigned long toc = millis();
  
//  Serial << " " << toc-tic << " ms." << endl;
  delay(100UL - (toc-tic));
  
}

unsigned long generateCode(uint16_t address, byte code) {
  unsigned long retCode=0;
  
//  Serial << "generateCode: address=" << _HEX(address) << " code=" << _HEX(code) << " complement=" << _HEX(255 & ~code) << endl;
  
  retCode |= ((unsigned long)address) << 16;
  retCode |= ((unsigned long)code) << 8;
  retCode |= (255 & ~code);
  
  return( retCode );
}

char* dec2binWzerofill(uint16_t Dec){
  char fill = '0';
  static char bin[16];
  unsigned int i=0;

  while (Dec > 0) {
    bin[16+i++] = ((Dec & 1) > 0) ? '1' : fill;
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j< 16; j++) {
    if (j >= 16 - i) {
      bin[j] = bin[ 15 + i - (j - (16 - i)) ];
    }else {
      bin[j] = fill;
    }
  }
  bin[16] = '\0';
  
  return bin;
}

