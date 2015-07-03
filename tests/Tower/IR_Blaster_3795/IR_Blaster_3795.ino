// need to strike RobotIRRemote directory in Arduino IDE libraries folder.  Name collision!
#include <SoftwareSerial.h>
#include <Streaming.h>
#include <Simon_Common.h> // I_RED, etc.
#include <Metro.h>

class IRlight {
  public:
    void begin( uint16_t address, byte sendCount, 
                byte on, byte off, byte up, byte down,
                byte red, byte green, byte blue, byte yellow, byte white,
                byte flash, byte strobe, byte fade, byte smooth );
         
    void on(), off(), up(), down();
    void flash(), strobe(), fade(), smooth();
    void color(byte color);
    
  private:
    void send(byte code);
    unsigned long generateCode(byte code);
    
    boolean isOn;
    
    uint16_t addressC;
    byte onC, offC, upC, downC, redC, greenC, blueC, yellowC, whiteC, flashC, strobeC, fadeC, smoothC;
    byte sendCount;
    byte currentColor;
    
};
  
SoftwareSerial IRsend(8, 7);

IRlight flood; // Loftek (50W) and off-brand (10W) floods, and RGB strip lighting.
IRlight miniSubs; // cute little CR2032s
IRlight packSubs; // pack-of-four submersibles

// about 70 ms to send packet

void setup()
{
  Serial.begin(115200);
  Serial << "Startup." << endl;
  
  IRsend.begin(9600);

  // minisubs 
  miniSubs.begin(0x01FE, 1, 0x48, 0x58, 0x48, 0x48, 0x20, 0xA0, 0x60, 0x50, 0x30, 0xC0, 0xC0, 0xC0, 0xC0); 
  // no up and down
 // strictly address 0x01FE
  
//  soondarSubs.begin(0x00FF, 0xA2, 0xE2, 0x52, 0x67, 0x22, 0x02, 0xC2, 0xA8, 0x10, 0x5A, 0x5A, 0x38, 0x38);
// no strobe and smooth
  
  // pack of submersibles 
  packSubs.begin(0x00FF, 1, 0xB0, 0xF8, 0x90, 0xB8, 0x98, 0xD8, 0x88, 0x38, 0xA8, 0xB2, 0x00, 0x58, 0x30 );
//  packSubs.begin(0x0018, 2, 0xB0, 0xF8, 0x90, 0xB8, 0x98, 0xD8, 0x88, 0x38, 0xA8, 0xB2, 0x00, 0x30, 0xB2 );
//  packSubs.begin(0x00F0, 0xB0, 0xF8, 0x90, 0xB8, 0x98, 0xD8, 0x88, 0x38, 0xA8, 0xB2, 0x00, 0x30, 0xB2 );
  // addresses that work end in 1, 8 and C. 0x00FF, too.
  // 0x???1, 0x???8, 0x???C and 0x00FF.
  //  flood.color(color); 

  // strips, small floods, big floods
  flood.begin(0x00F7, 1,   0xC0, 0x40, 0x00, 0x80, 0x20, 0xA0, 0x60, 0x28, 0xE0, 0xD0, 0xF0, 0xC8, 0xE8);
//  for( byte i=0; i<10; i++) flood.up();
  // 50W floods: any damn thing
  // strips: 0x00F?
  // 10W floods: 0x??F7
  
  Serial << "On." << endl;
  setOn();
  setOn();
  setOn();
  
  Serial << F("Free RAM: ") << freeRam() << endl;
}

/*

obey address codes: strips, small floods, minisubs
DON'T obey address codes: pack subs, big floods

*/


void setColor(byte color) {
  miniSubs.color(color); 
  packSubs.color(color); 
  flood.color(color); 
}

void setFade() {
  miniSubs.fade(); 
  packSubs.fade();
  flood.fade(); 
}

void setOn() {
  miniSubs.on(); 
  packSubs.on(); 
  flood.on(); 
}


void loop() {
  
  const char *c[]={"Red","Green","Blue","Yellow","White"};
  static byte color = 0;
  
  Serial << c[color] << endl;
  setColor(color);
  delay(3000);
   
//  Serial << "Fade" << endl;
//  setFade();
//  delay(3000);
  
  (++color) %= 5;
  
}

void IRlight::begin(uint16_t address, byte sendCount,
                byte on, byte off, byte up, byte down,
                byte red, byte green, byte blue, byte yellow, byte white,
                byte flash, byte strobe, byte fade, byte smooth ) {
  // with this information set up codes
  
  this->addressC = address;
  this->sendCount = sendCount;
  
  this->onC = on;
  this->offC = off;
  this->upC = up;
  this->downC = down;
  
  this->redC = red;
  this->greenC = green;
  this->blueC = blue;
  this->yellowC = yellow;
  this->whiteC = white;
  
  this->flashC = flash;
  this->strobeC = strobe;
  this->fadeC = fade;
  this->smoothC = smooth;
  
  this->currentColor = 255; // not set
  this->isOn = false;
  
}

void IRlight::color(byte color) {
  
  if( this->isOn && color == currentColor ) return; // no need.
  
  // track
  currentColor = color;
  
//  if( !this->isOn ) this->on();
  
  switch( color ) {
    case I_RED: send(redC); break;
    case I_GRN: send(greenC); break;
    case I_BLU: send(blueC); break;
    case I_YEL: send(yellowC); break;
    default: send(whiteC); break;
  }
  
//  delay(100);
      
}

void IRlight::on() { send(this->onC); this->isOn=true; }
void IRlight::off() { send(this->offC); this->isOn=false; }
void IRlight::up() { send(this->upC); }
void IRlight::down() { send(this->downC); }
void IRlight::flash() { send(this->flashC); }
void IRlight::strobe() { send(this->strobeC); }
void IRlight::fade() { send(this->fadeC); }
void IRlight::smooth() { send(this->smoothC); }

void IRlight::send(byte code) {

  static Metro sendInterval(100UL); // ms
  
  while( !sendInterval.check() ); // wait for last send to cle
  
  Serial << "sending at " << millis() << endl;
  
  for( byte i=0; i<sendCount; i++ ) {
  
    Serial << _HEX(0xCC) << " " << _HEX(lowByte(this->addressC)) << " " << _HEX(highByte(this->addressC)) << " " << _HEX(code) << endl;
    
    IRsend.write(0xCC);
    IRsend.write(lowByte(this->addressC));
    IRsend.write(highByte(this->addressC));
    IRsend.write(code);
  
    sendInterval.reset();  
  }

}

unsigned long IRlight::generateCode(byte code) {
  unsigned long retCode=0;
  
//  Serial << "generateCode: address=" << _HEX(address) << " code=" << _HEX(code) << " complement=" << _HEX(255 & ~code) << endl;
  
  retCode |= ((unsigned long)this->addressC) << 16;
  retCode |= ((unsigned long)code) << 8;
  retCode |= (255 & ~code);
  
  return( retCode );
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

