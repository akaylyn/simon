#include <SoftwareSerial.h>
#include <Streaming.h>
#include <Metro.h>

SoftwareSerial IRsend(8, 7); // Pin 7 to ST3795 RXD (pin 3)

uint16_t addressC = 0x00F7;
byte redC = 0x20;
byte grnC = 0xA0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial << "Startup." << endl;
  
  IRsend.begin(9600);


}

void loop() {
  // put your main code here, to run repeatedly: 
  send(redC);
  delay(1000);
  send(grnC);
  delay(1000);
  
  
}

void send(byte code) {

  static Metro sendInterval(100UL); // ms
  
  while( !sendInterval.check() ); // wait for last send to clear
  
  Serial << "sending at " << millis() << endl;
  
  Serial << _HEX(0xCC) << " " << _HEX(lowByte(addressC)) << " " << _HEX(highByte(addressC)) << " " << _HEX(code) << endl;
    
  IRsend.print(0xCC);
  IRsend.print(lowByte(addressC));
  IRsend.print(highByte(addressC));
  IRsend.print(code);
  
  sendInterval.reset();  
 
}

