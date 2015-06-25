// need to strike RobotIRRemote directory in Arduino IDE libraries folder.  Name collision!
#include <IRremote.h>
#include <Streaming.h>
#include <Simon_Common.h> // I_RED, etc.

#define N_CODE 3
const unsigned long codeOff[N_CODE] =    {0xF740BF, 0xFF609F, 0xFFF807};
const unsigned long codeOn[N_CODE] =     {0xF7C03F, 0xFFE01F, 0xFFB04F};
const unsigned long codeRed[N_CODE] =    {0xF720DF, 0xFF906F, 0xFF9867}; // collision with codeUp!
const unsigned long codeGreen[N_CODE] =  {0xF7A05F, 0xFF10EF, 0xFFD827};
const unsigned long codeBlue[N_CODE] =   {0xF7609F, 0xFF50AF, 0xFF8877};
const unsigned long codeYellow[N_CODE] = {0xF728D7, 0xFF8877, 0xFF38C7};
const unsigned long codeWhite[N_CODE] =  {0xF7E01F, 0xFFD02F, 0xFFA857};
const unsigned long codeDown[N_CODE] =   {0xF7807F, 0xFF20DF, 0xFFB847};
const unsigned long codeUp[N_CODE] =     {0xF700FF, 0xFFA05F, 0xFF906F};
const unsigned long codeFlash[N_CODE] =  {0xF7D02F, 0xFFF00F, 0xFFB24D};
const unsigned long codeStrobe[N_CODE] = {0xF7F00F, 0xFFE817, 0xFF00FF};
const unsigned long codeFade[N_CODE] =   {0xF7C837, 0xFFD827, 0xFF58A7}; // collision with codeGreen!
const unsigned long codeSmooth[N_CODE] = {0xF7E817, 0xFFC837, 0xFF30CF};

class Flood {
  public:
    void begin();
    
    void setColor(byte color);
      
    void send(const unsigned long codes[]);
  private:
 
    byte currentColor;
    
    IRsend irsend;
};

Flood flood;


void setup()
{
  Serial.begin(115200);
  Serial << "Startup." << endl;
  
  flood.begin(); 

}

void loop() {
  
  Serial << F("Free RAM: ") << freeRam() << endl;
  Serial << "On." << endl;
  flood.send(codeOn);
 
  Serial << "Red." << endl;
  flood.setColor(I_RED); delay(5000);
  Serial << "Green." << endl;
  flood.setColor(I_GRN); delay(5000);
  Serial << "Blue." << endl;
  flood.setColor(I_BLU); delay(5000);
  Serial << "Yellow." << endl;
  flood.setColor(I_YEL); delay(5000);
  Serial << "White." << endl;
  flood.setColor(N_COLORS); delay(5000); // white
  
}

void Flood::begin() {
  // on
  flood.send(codeOn);
  flood.send(codeOn);

  // very bright
  flood.send(codeUp);
  flood.send(codeUp);
  flood.send(codeUp);
  flood.send(codeUp);
  flood.send(codeUp);
  flood.send(codeUp);
  flood.send(codeUp);
  flood.send(codeUp);
    
  Serial << F("Flood: on.") << endl;
}

void Flood::setColor(byte color) {
  // track
  currentColor = color;
  
  switch( color ) {
    case I_RED: send(codeRed); break;
    case I_GRN: send(codeGreen); break;
    case I_BLU: send(codeBlue); break;
    case I_YEL: send(codeYellow); break;
    default: send(codeWhite); break;
  }
      
}


void Flood::send(const unsigned long codes[]) {
  for( int c=N_CODE-1; c>=0; c-- ) {
    Serial << "Send: " << _HEX(codes[c]) << endl;
    
    // simple wrapper
    irsend.sendNEC(codes[c], 32);
    for( byte i=1; i < 2; i++ ) {
      delay(8); // need a 8ms interval before resend
      irsend.sendNEC(codes[c], 32);
    }
    delay(20);
  }
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

