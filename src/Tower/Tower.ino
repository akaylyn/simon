// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <RFM12B.h> // RFM12b radio transmitter module
#include <EEPROM.h> // saving and loading radio settings
#include <Simon.h> // sizes, indexing and comms common to Towers and Console

// this is where the lights and fire instructions from Console are placed
towerInstruction inst;
// structure stores how this Tower acts on those towerInstructions
towerConfiguration config;

// for yellow on a RGB strip, we need to active red and green in some proportion.
// from testing, the red should be 74% and green 26%.
// So, if run the red strip at the luminance requested
// for yellow, then the green strip needs to be 0.35 = the ratio 0.26/0.74.  
const byte greenYellowFraction = 35;

// store my NODEID
byte myNodeID;
// store an in-network state
boolean inNetwork = false;

// track how long we need to have the flame on for.
Metro flameOnTime(100UL);
// but only fire this often.
Metro flameCoolDownTime(2000UL);

// pin locations for outputs
#define LED_R 6 // the PWM pin which drives the red LED
#define LED_G 9 // the PWM pin which drives the green LED
#define LED_B 5 // the PWM pin which drives the blue LED
#define FLAME 4 // JeeNode Port 1, pin 2 (DIO)
#define OTHER 14 // JeeNode Port 1, pin 5 (AIO)
#define GPIO1 7 // JeeNode Port 4, pin 2 (DIO)
#define GPIO2 17 // JeeNode Port 4, pin 5 (AIO)

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial << F("Configuring output pins.") << endl;
  flameOff(); pinMode(FLAME, OUTPUT); // order is important.  set low, then output.
  otherOff(); pinMode(OTHER, OUTPUT); // order is important.  set low, then output.
  digitalWrite(LED_R, LOW); pinMode(LED_R, OUTPUT);
  digitalWrite(LED_G, LOW); pinMode(LED_G, OUTPUT);
  digitalWrite(LED_B, LOW); pinMode(LED_B, OUTPUT);

  myNodeID = commsStart(); // assume that RF12Demo has been used to correctly set values
  if ( myNodeID == 0 ) {
    Serial << F("Unable to recover RFM settings!") << endl;
    while (1);
  }

  // set default configuration
  commsDefault(config); commsPrint(config);
  commsDefault(inst); commsPrint(inst);

  // do them
  performInstruction();

  Serial << F("Startup complete. Node: ") << myNodeID << endl;
}


void loop() {
  // put your main code here, to run repeatedly:

  // SAFETY: do not move this code after any other code.
  if ( flameOnTime.check() ) { // time to turn the flame off
    flameOff();
  }

  if ( commsRecv(inst) ) { // did we get instruction from Console?

    performInstruction(); // if yes, execute those instructions, ASAP.
    
  } else if ( commsRecv(config) ) { // or, did we get configuration from Console?

    inNetwork = true; // now we know we're in network, and can stop processing out-of-network test signal
  
    commsPrint(config); // show the changes
    
    flameCoolDownTime.interval(config.flameCoolDownTime); // set the flame cooldown
    
  } else if ( commsRecvPing() ){
    
    Serial << "P";
    
  } else if ( ! inNetwork ) { // otherwise we're on our own... lonely!
    
    if( updateTestPattern() ) // if we need to update the test pattern.
      performInstruction(); // do so.
    
  }

}

boolean updateTestPattern() {
  
  // gotta start somewhere
  static byte currInd = I_YEL;
  // track the last position in a cycle
  static float ltc = 1;
  // track the last value setting
  static byte lastval = 0;
  
  // return true if we need to update
  boolean needUpdate = false;
  
  // we'll use the commsInstruction instrument to simulate received instructions
  
  // use a breathing function.
  
  // total cycle time
  const unsigned long cycleLength = 2500; // ms.  2.5 second cycle
  // calculate where we are
  float tc = (millis() % cycleLength)/cycleLength; // 0..1
  // compare to the last tc
  if( tc<ltc ) { // enter a new cycle
    commsDefault(inst, 0, 0); // reset instructions, making sure fire is off.
    currInd = ++currInd % (N_COLORS+1); // select the next color
    needUpdate = true;
  }
  ltc = tc; // save current position
  byte val = (exp(sin(tc*2.0*PI + 0.75*2.0*PI)) - 0.36787944)*108.0;
  if( val != lastval || needUpdate ) { // if the value changed, or if we've flipped to a new color
    lastval = val;
    inst.lightLevel[currInd] = val;
    needUpdate = true;
  }
  
  return( needUpdate );
 
}

void performInstruction() {
  // process inst through the filter of config
  
  // colors can be set independently.
  if ( config.colorIndex == I_RED || config.colorIndex == I_ALL) {
    lightSet(LED_R, inst.lightLevel[I_RED]);
  }
  if ( config.colorIndex == I_GRN || config.colorIndex == I_ALL) {
    lightSet(LED_G, inst.lightLevel[I_GRN]);
  }
  if ( config.colorIndex == I_BLU || config.colorIndex == I_ALL) {
    lightSet(LED_B, inst.lightLevel[I_BLU]);
  }
  if ( config.colorIndex == I_YEL || config.colorIndex == I_ALL) {
    // get yellow by portion of red and green
    // NOTE: integer math used for speed.  floats are slow.
    lightSet(LED_R, inst.lightLevel[I_YEL]);
    lightSet(LED_G, (inst.lightLevel[I_YEL]*greenYellowFraction) / 100);
  }
  
  // fire is a little different, if we're "listening" to multiple fire channels.
  // assume we're not going to shoot fire.
  byte maxFireLevel = 0;
  // and then check for the longest requested fire length.
  if ( config.fireIndex == I_RED || config.fireIndex == I_ALL) {
    maxFireLevel = max(maxFireLevel, inst.fireLevel[I_RED]);
  }
  if ( config.fireIndex == I_GRN || config.fireIndex == I_ALL) {
    maxFireLevel = max(maxFireLevel, inst.fireLevel[I_GRN]);
  }
  if ( config.fireIndex == I_BLU || config.fireIndex == I_ALL) {
    maxFireLevel = max(maxFireLevel, inst.fireLevel[I_BLU]);
  }
  if ( config.fireIndex == I_YEL || config.fireIndex == I_ALL) {
    maxFireLevel = max(maxFireLevel, inst.fireLevel[I_YEL]);
  }
  // execute on the longest requested flame length.
  flameOn(maxFireLevel);

}

void lightSet(int lightPin, byte lightLevel) {
  if( lightLevel <= 0 ) { // special case for fireLevel==0 i.e. "none"
    digitalWrite(lightPin, LOW); // also, faster than analogWrite
  } else if ( lightLevel >= 255 ) { // special case for fireLevel==255 i.e. "full"
    digitalWrite(lightPin, HIGH); // also, faster than analogWrite
  } else {
    analogWrite(lightPin, lightLevel); // analogWrite for everything between
  }
}

void flameOn(int fireLevel) {

  // special case for fireLevel==0 i.e. "none"
  if( fireLevel == 0 ) return;
  
  // only if the cooldown timer is ready
  if ( flameCoolDownTime.check() ) {

    unsigned long flameTime = constrain( // constrain may be overkill, but map doesn't guarantee constraints
                                map(fireLevel, 1, 255, config.minFireTime, config.maxFireTime),
                                config.minFireTime, config.maxFireTime);

    // SAFETY: never, ever, ever set this pin high without resetting the flameOnTime timer, unless you
    // have some other way of turning it back off.
    // To put a Fine Point on it: this simple line will unleash 100,000 BTU.  Better take care.

    digitalWrite(FLAME, HIGH); // This line should appear exactly once in a sketch, and be swaddled in all manner of caution.

    flameOnTime.interval(flameTime);
    flameOnTime.reset();
  }
}

void flameOff() {
  // start cooldown counter
  flameCoolDownTime.reset();

  digitalWrite(FLAME, LOW);
}

void otherOn() {
  digitalWrite(OTHER, HIGH);
}

void otherOff() {
  digitalWrite(OTHER, LOW);
}
