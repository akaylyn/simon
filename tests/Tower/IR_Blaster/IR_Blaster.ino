// need to strike RobotIRRemote directory in Arduino IDE libraries folder.  Name collision!
#include <IRremote.h>
#include <Streaming.h>
#include <Simon_Common.h> // I_RED, etc.

#define K24_OFF          0xF740BF
#define K24_ON           0xF7C03F

#define K24_RED          0xF720DF
#define K24_GRN          0xF7A05F
#define K24_BLU          0xF7609F
#define K24_YEL          0xF728D7
#define K24_WHT          0xF7E01F

#define K24_DOWN         0xF7807F
#define K24_UP           0xF700FF

#define K24_FLASH        0xF7D02F
#define K24_STROBE       0xF7F00F
#define K24_FADE         0xF7C837      
#define K24_SMOOTH       0xF7E817

#define K24_REPEAT       0xFFFFFF

#define N_BRIGHT_STEPS   5

class Flood {
  public:
    void begin();
    
    void setBright(byte level);
    void setColor(byte color);
  
    
    void send(unsigned long data);
    void sendMultiple(unsigned long data, byte repeats);
    
    IRsend irsend;
    
  protected:
    int currentColor;
    int currentBright;    

};

Flood flood;


void setup()
{
  Serial.begin(115200);
  Serial << "Startup." << endl;
  
  // shut it all the way down
  flood.sendMultiple(K24_OFF, 4);
  
  flood.begin();
 
}

void loop() {
  
  Serial << F("Free RAM: ") << freeRam() << endl;
 
  static byte bright=0;
  bright+=1;
  if( bright> N_BRIGHT_STEPS) bright = 0;
  
  flood.setBright(bright);
  
  flood.setColor(I_RED); delay(1000);
  flood.setColor(I_GRN); delay(1000);
  flood.setColor(I_BLU); delay(1000);
//  flood.setColor(I_YEL); delay(1000);
//  flood.setColor(99); delay(1000);
  
}

void Flood::begin() {
  // on
  sendMultiple(K24_ON, 2);
  
  // gotta start somewhere
  setColor(99);
  currentBright = 0;
  setBright(N_BRIGHT_STEPS);
  
  Serial << F("Flood: on.") << endl;
}

void Flood::setColor(byte color) {
  // track
  currentColor = color;
  
  switch( color ) {
    case I_RED: send(K24_RED); break;
    case I_GRN: send(K24_GRN); break;
    case I_BLU: send(K24_BLU); break;
    case I_YEL: send(K24_YEL); break;
    default: send(K24_WHT); break;
  }
      
}

void Flood::setBright(byte level) {
  if( level > N_BRIGHT_STEPS ) level = N_BRIGHT_STEPS;

  Serial << "Bright: " << level << endl;
  
  byte diff = abs(level-currentBright);
  if( level > currentBright ) {
    sendMultiple(K24_UP, diff);
  } else if( level < currentBright) {
    sendMultiple(K24_DOWN, diff);
  }
  // track
  currentBright = level;
}

void Flood::send(unsigned long data) {
  // simple wrapper
  irsend.sendNEC(data, 32);
}

void Flood::sendMultiple(unsigned long data, byte repeats) {
  send(data); 
  for( byte i=1; i < repeats; i++ ) {
    delay(8); // need a 8ms interval before resend
    send(data); 
//    send(K24_REPEAT);
  }
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
