//#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Streaming.h>
#include <Metro.h>

// GRN > RED
//  ^     v
// YEL < BLU

#define RIM_PIN 13 
#define RIM_X 20 
#define RIM_Y 3 
#define RIM_SEG_LENGTH RIM_X/4 // floor(RIM_N/4)=27
#define RED_OFFSET 12
#define RED_X (RED_OFFSET + RIM_SEG_LENGTH) % RIM_X
#define BLU_X (RED_X + RIM_SEG_LENGTH) % RIM_X
#define YEL_X (BLU_X + RIM_SEG_LENGTH) % RIM_X
#define GRN_X (YEL_X + RIM_SEG_LENGTH) % RIM_X
#define ALL_Y 1

#define UPDATE_INTERVAL 100UL
Metro updateCycles(UPDATE_INTERVAL);

#define ADD_INTERVAL 1000UL
Metro addCycles(ADD_INTERVAL);

Adafruit_NeoPixel rimJob = Adafruit_NeoPixel(RIM_X*RIM_Y, RIM_PIN, NEO_GRB + NEO_KHZ800);

const uint32_t Red = rimJob.Color(255, 0, 0);
const uint32_t Yel = rimJob.Color(255, 255, 0);
const uint32_t Grn = rimJob.Color(0, 255, 0);
const uint32_t Blu = rimJob.Color(0, 0, 255);
const uint32_t Dead = rimJob.Color(0, 0, 0);
const uint32_t White = rimJob.Color(255, 255, 255);

// tracks the cycles
typedef struct {
  boolean live; // live or dead
  uint32_t x,y; // location
  uint32_t color; // color of the cycle
} cycle_t;

#define MAX_CYCLES 20
cycle_t cycles[MAX_CYCLES];

void setup() {
  
  Serial.begin(115200);
  
  // put your setup code here, to run once:
  // Neopixel strips
  rimJob.begin();

  // clear strip
  for( int p=0; p<rimJob.numPixels(); p++ ) rimJob.setPixelColor(p, Dead);
  
  Serial << F("numPixels: ") << rimJob.numPixels() << endl;

  // clear cycles
  for( int c=0; c<MAX_CYCLES; c++ ) {
    cycles[c].live=false;
  }
  
  addCycle(RED_X, ALL_Y, Red);
  addCycle(GRN_X, ALL_Y, Grn);
  addCycle(BLU_X, ALL_Y, Blu);
  addCycle(YEL_X, ALL_Y, Yel);
  
  Serial << F("Startup complete.") << endl;
  
  rimJob.show();
  serialPrint();

}

void loop() {
  // put your main code here, to run repeatedly:
  if( updateCycles.check() ) {
    fadeCycles();
    moveCycles();
    serialPrint();
    rimJob.show();
  }
  if( addCycles.check() ) { 
    switch( random(0,4) ) {
      case 0: addCycle(RED_X, ALL_Y, Red); break;
      case 1: addCycle(GRN_X, ALL_Y, Grn); break;
      case 2: addCycle(BLU_X, ALL_Y, Blu); break;
      case 3: addCycle(YEL_X, ALL_Y, Yel); break;
    }
  }
}

void serialPrint() {
  static unsigned long it=0;
  Serial << endl << it << F(":") << endl;
  for( int y=0; y<RIM_Y; y++) {
    for( int x=0; x<RIM_X; x++) {
      uint32_t c = rimJob.getPixelColor(getPixelN(x,y));
      boolean isC = isCycle(x,y);
      char p;
      
           if( isC && c==Red ) p = 'R';
      else if( isC && c==Grn ) p = 'G';
      else if( isC && c==Blu ) p = 'B';
      else if( isC && c==Yel ) p = 'Y';
      else if( isC ) p = '!'; // BAD: cycles have a color
      else if( c==Red ) p = 'r';
      else if( c==Grn ) p = 'g';
      else if( c==Blu ) p = 'b';
      else if( c==Yel ) p = 'y';
      else if( c==Dead ) p = ' ';
      else if( c==White ) p = '*';
      else p = '.'; // faded/cycle trail
      
      Serial << p;
    }
    Serial << endl;
  }
   
  it++;
}

// looks for active cycle at a pixel location
boolean isCycle(int x, int y) {
  for( int c=0; c<MAX_CYCLES; c++ ) {
    if( cycles[c].live && cycles[c].x==x && cycles[c].y==y ) 
      return( true );
  }
  return( false );
}

void addCycle(uint32_t x, uint32_t y, uint32_t color) {
  
  byte availableCycle=0;
  while( cycles[availableCycle].live ) { 
    availableCycle++;
    if( availableCycle == MAX_CYCLES ) {
      Serial << F("addCycle error.  no available cycle slots.  Halting!") << endl;
      while(1);
    }
  }
  
  rimJob.setPixelColor(getPixelN(x,y), color);
  
  cycles[availableCycle].x = x;
  cycles[availableCycle].y = y;
  cycles[availableCycle].color = color;
  cycles[availableCycle].live = true;
}

void fadeCycles() {
  const int trailLength = RIM_SEG_LENGTH; // a light cycle leaves a trail as long as 1/4 of the rim circumference
  const int fadeAmount = 255 / trailLength;
  
  for( int p=0; p<rimJob.numPixels(); p++ ) {
    uint32_t c = rimJob.getPixelColor(p);
    
    // assuming GRB order.
    byte red = constrain( (int)((c << 16) >> 24) - fadeAmount, 0, 255);
    byte green = constrain( (int)((c << 8) >> 24) - fadeAmount, 0, 255);
    byte blue = constrain( (int)((c << 24) >> 24) - fadeAmount, 0, 255);
    
    rimJob.setPixelColor(p, red, green, blue);
  }
}

void moveCycles() {
  for( byte c=0; c<MAX_CYCLES; c++ ) {
    if( cycles[c].live ) {    
      moveThisCycle(c);
     }
  }
}

void moveThisCycle(byte c) {
  int cw = (int)cycles[c].x+1;
  if( cw<0 ) cw = RIM_X-1;
  else if ( cw >= RIM_X) cw = 0;
  
  int ccw = (int)cycles[c].x-1;
  if( ccw<0 ) ccw = RIM_X-1;
  else if ( ccw >= RIM_X) ccw = 0;

  uint32_t moveX[4] = {
    cw, // CW. wrap.
    ccw, // CCW. wrap
    cycles[c].x, // DOWN
    cycles[c].x, // UP  
  };
  uint32_t moveY[4] = {
    cycles[c].y, // CW
    cycles[c].y, // CCW
    constrain(((int)cycles[c].y+1), 0, RIM_Y-1), // DOWN.  no wrap.
    constrain(((int)cycles[c].y-1), 0, RIM_Y-1), // UP. no wrap.
  };
  
  // preference for moves
  byte movePref[4];
/*
  // prefer CW/CCW over UP/DOWN
  movePref[0]=random(0,2); // either CW or CCW.
  movePref[1]=movePref[0]==0 ? 1 : 0;
  movePref[2]=random(2,4); // either UP or DOWN.
  movePref[3]=movePref[2]==2 ? 3 : 2;
*/
  // equal preference
  movePref[0]=random(0,4); // any
  movePref[1]=(movePref[0]+1) % 4;
  movePref[2]=(movePref[1]+1) % 4;
  movePref[3]=(movePref[2]+1) % 4;

  // try some moves
  for( byte m=0; m<4; m++ ) {
    if( rimJob.getPixelColor(getPixelN(moveX[movePref[m]], moveY[movePref[m]])) == Dead ) {
      // good.
      cycles[c].x = moveX[movePref[m]];
      cycles[c].y = moveY[movePref[m]];
      
      rimJob.setPixelColor(getPixelN(cycles[c].x,cycles[c].y), cycles[c].color);
      
      return;
    }
  }
  // uh oh. smash!
  cycles[c].live = false;
  rimJob.setPixelColor(getPixelN(cycles[c].x,cycles[c].y), White);

/*  
  Serial << "Oh dear! Cycle " << c << " died." << endl;
  Serial << "Moves available: " << endl;
  for( byte m=0; m<4; m++ ) {
    Serial << "X=" << moveX[movePref[m]] << " Y=" << moveY[movePref[m]] << endl;
  }
  Serial << "Result: ";
  */
}

// returns the i-th pixel mapped to x,y
uint32_t getPixelN(uint32_t x, uint32_t y) {
  if( x >= RIM_X || y >= RIM_Y ) {
    Serial << F("getPixelN error: out of range.  x=") << x << F(" y=") << y << endl << F("Halting.") << endl;
    while(1);
  }
  return( y*RIM_X + x ); // this could be right?
}
