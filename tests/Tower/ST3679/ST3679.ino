#include <SoftwareSerial.h>
#include <Streaming.h>
#include <Metro.h>

SoftwareSerial IRrecv(A0, A1); // Pin 7 to ST3795 RXD (pin 3)

uint16_t addressC = 0x00F7;
byte redC = 0x20;
byte grnC = 0xA0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial << "Startup." << endl;
  
  IRrecv.begin(9600);

  Serial << "On: 0xC0 " << _BIN(0xC0) << endl;
  Serial << "Off: 0x40 " << _BIN(0x40) << endl;
  Serial << "Down: 0x80 " << _BIN(0x80) << endl;
  Serial << "?: 0xFF " << _BIN(0xFF) << endl;
}

/* From Sunrom's garbage datasheet and a piece of example code I found for a related product of theirs:

// extract rc5 data from serial buffer to 16 bit value
rc5data = ((sbuffer[2]-'0')<<4)|(sbuffer[3]-'0'); // convert data from ASCII to low byte
rc5data |= (((sbuffer[0]-'0')<<4)|(sbuffer[1]-'0'))<<8; // convert data from ASCII to high byte					
					
*/

void loop() {
  byte b1,b2,b3,b4,b5,b6;
  uint8_t addr, mesg;
  if( IRrecv.available() >=6 ) {
    b1 = IRrecv.read()-0x30;
    b2 = IRrecv.read()-0x30;
    b3 = IRrecv.read()-0x30;
    b4 = IRrecv.read()-0x30;
    IRrecv.read(); // dump
    IRrecv.read(); // dump
    Serial << " b1:" << _HEX(b1) << " " << _BIN(b1);
    Serial << " b2:" << _HEX(b2) << " " << _BIN(b2);
    Serial << " b3:" << _HEX(b3) << " " << _BIN(b3);
    Serial << " b4:" << _HEX(b4) << " " << _BIN(b4);
    Serial << endl;
    
    addr = b2 | (b1<<4);
    
    Serial << "addr:" << _HEX(addr) << " " << _BIN(addr) << endl;

    mesg = bitReverse(b4 | (b3<<4));
    
    Serial << "mesg:" << _HEX(mesg) << " " << _BIN(mesg) << endl;
    
  }
}

// Reverse the order of bits in a byte. 
// I.e. MSB is swapped with LSB, etc. 
unsigned char bitReverse( unsigned char x ) 
{ 
   x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa); 
   x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc); 
   x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0); 
   return x;    
}

