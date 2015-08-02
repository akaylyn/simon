// Compile for Arduino Pro/Pro Mini 5V 16Mhz with ATMega 328

// need to strike RobotIRRemote directory in Arduino IDE libraries folder.  Name collision!
#include <IRremote.h> // IR connected to pin 3
#include <Streaming.h>
#include <Metro.h>
#include <Simon_Common.h> // I_RED, etc.
#include <EasyTransfer.h> // rx, tx
#include <SoftwareSerial.h> // 

SoftwareSerial SSerial(A2, A3); // to A2 and A3 on Tower Moteino, swapping the cabling
EasyTransfer ET; 
colorInstruction lastColorInst, newColorInst;

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
  
IRsend irsend;

IRlight flood; // Loftek (50W) and off-brand (10W) floods, and RGB strip lighting.
IRlight miniSubs; // cute little CR2032s
IRlight packSubs; // pack-of-four submersibles

// about 70 ms to send packet

void setup()
{
  Serial.begin(115200);
  Serial << "Startup." << endl;

  SSerial.begin(9600);
  ET.begin(details(newColorInst), &SSerial);

  // LED for notable sending
  pinMode(13, OUTPUT);

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

// first: obey address codes: strips, small floods, minisubs
// last: DON'T obey address codes: pack subs, big floods
void setColor(byte color) {
  Serial << F("Instruction, color: ") << color << endl;

  if( !ET.receiveData() ) miniSubs.color(color); 
  if( !ET.receiveData() ) packSubs.color(color); 
  if( !ET.receiveData() ) flood.color(color); 
}
void setFade() {
  Serial << F("Instruction, fade.")<< endl;

  if( !ET.receiveData() ) miniSubs.fade(); 
  if( !ET.receiveData() ) packSubs.fade();
  if( !ET.receiveData() ) flood.fade(); 
}
void setOn() {
  Serial << F("Instruction, on.")<< endl;

  if( !ET.receiveData() ) miniSubs.on(); 
  if( !ET.receiveData() ) packSubs.on(); 
  if( !ET.receiveData() ) flood.on(); 
}
void setOff() {
  Serial << F("Instruction, off.")<< endl;

  if( !ET.receiveData() ) miniSubs.off(); 
  if( !ET.receiveData() ) packSubs.off(); 
  if( !ET.receiveData() ) flood.off(); 
}

void loop() {
  
 // check SSerial
  if( ET.receiveData() ) {
    // have data.  is it different than last?
    if( memcmp((void*)(&newColorInst), (void*)(&lastColorInst), sizeof(colorInstruction)) != 0 ) {
      if( newColorInst.red > 0 && newColorInst.green > 0 && newColorInst.blue > 0 ) setColor(4); // white
      else if( newColorInst.red > 0 && newColorInst.green > 0 ) setColor(I_YEL);
      else if( newColorInst.red > 0 ) setColor(I_RED);
      else if( newColorInst.green > 0 ) setColor(I_GRN);
      else if( newColorInst.blue > 0 ) setColor(I_BLU);
      else setOff();
      
      lastColorInst = newColorInst;
    }    
  } 

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
  
  // turn on.
  if( !this->isOn ) this->on();

  if( this->isOn && color == currentColor ) return; // no need.
  
  // track
  currentColor = color;
  
  switch( color ) {
    case I_RED: send(redC); break;
    case I_GRN: send(greenC); break;
    case I_BLU: send(blueC); break;
    case I_YEL: send(yellowC); break;
    default: send(whiteC); break;
  } 

}

void IRlight::on() { if( !this->isOn ) send(this->onC); this->isOn=true; }
void IRlight::off() { if( this->isOn ) send(this->offC); this->isOn=false; }
void IRlight::up() { send(this->upC); }
void IRlight::down() { send(this->downC); }
void IRlight::flash() { send(this->flashC); }
void IRlight::strobe() { send(this->strobeC); }
void IRlight::fade() { send(this->fadeC); }
void IRlight::smooth() { send(this->smoothC); }

void IRlight::send(byte code) {
  unsigned long msg = generateCode(code);
  Serial << "Send: " << _HEX(msg);
  unsigned long tic, toc;
  
  for( byte i=0; i<sendCount; i++ ) {
    tic = millis();
    digitalWrite(13, HIGH);
    irsend.sendNEC(msg, 32);
    digitalWrite(13, LOW);
    toc = millis();
    delay(100UL - (toc-tic));
  }
  Serial << " " << toc-tic << " ms. x" << sendCount << endl;

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

