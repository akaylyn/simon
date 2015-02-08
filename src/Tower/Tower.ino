// Compile for Uno.

// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Bounce.h> // buttons
#include <Metro.h> // timers
#include <Simon_Indexes.h> // sizes, indexing and comms common to Towers and Console
#include <EEPROM.h> // saving and loading radio settings
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module
#include <Simon_Comms.h>  // Tower<=>Console

// this is where the lights and fire instructions from Console are placed
towerInstruction inst;
// structure stores how this Tower acts on those towerInstructions
towerConfiguration config;

// for yellow on a RGB strip, we need to active red and green in some proportion.
// from testing, the red should be 74% and green 26%.
// So, if run the red strip at the luminance requested
// for yellow, then the green strip needs to be 0.35 = the ratio 0.26/0.74.
const byte greenYellowFraction = 35;

// Need an instance of the Radio Module
RFM69 radio;
// store my NODEID
byte myNodeID;
// keep track of activity to note network drop out
#define D_NETWORK_TIMOUT 10000UL
Metro networkTimeout(D_NETWORK_TIMOUT, 0); 

boolean amConfigured = false; // ask for config once after startup.
boolean doTestPatterns = true; // switch to test patterns

// track how long we need to have the flame on for.
Metro flameOnTime(100UL);
// but only fire this often.
Metro flameCoolDownTime(2000UL);

// when we're out of the network, update the lights on this interval (approx)
#define SOLO_TEST_PATTERN_UPDATE 3000UL // ms

// when we're in the network and idle, update the lights on this interval
#define NETWORK_TEST_PATTERN_UPDATE 20UL // ms

// pin locations for outputs
#define LED_R 3 // the PWM pin which drives the red LED
#define LED_G 5 // the PWM pin which drives the green LED
#define LED_B 6 // the PWM pin which drives the blue LED
#define LED_W 9 // the PWM pin which drives the blue LED
#define FLAME 7 // JeeNode Port 1, pin 2 (DIO)
#define OTHER 8 // JeeNode Port 1, pin 5 (AIO)

// minimum value on the LEDs that don't flicker
#define MIN_PWM 3

// luminosity is not equal among LEDs
#define RED_MAX 255
#define GRN_MAX 220
#define BLU_MAX 210

// remote control
#define SYSTEM_ENABLE_PIN A0
#define FIRE_ENABLE_PIN A1
Bounce systemEnable = Bounce(SYSTEM_ENABLE_PIN, BUTTON_DEBOUNCE_TIME);
Bounce fireEnable = Bounce(FIRE_ENABLE_PIN, BUTTON_DEBOUNCE_TIME);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(0));

  // see: http://jeelabs.org/2011/11/09/fixing-the-arduinos-pwm-2/
  bitSet(TCCR1B, WGM12);

  Serial << F("Configuring output pins.") << endl;
  flameOff(); pinMode(FLAME, OUTPUT); // order is important.  set then output.
  otherOff(); pinMode(OTHER, OUTPUT); // order is important.  set then output.
  digitalWrite(LED_R, LOW); pinMode(LED_R, OUTPUT);
  digitalWrite(LED_G, LOW); pinMode(LED_G, OUTPUT);
  digitalWrite(LED_B, LOW); pinMode(LED_B, OUTPUT);

  // once, at hardware initialization, we need to bootstrap some settings to the EEPROM
  //  commsSave(consoleNodeID); // write to EEPROM. Select consoleNodeID.
  commsSave(towerNodeID[0]); // write to EEPROM. Select towerNodeID[0..3].
//done  commsSave(towerNodeID[1]); // write to EEPROM. Select towerNodeID[0..3].
//done  commsSave(towerNodeID[2]); // write to EEPROM. Select towerNodeID[0..3].
//done  commsSave(towerNodeID[3]); // write to EEPROM. Select towerNodeID[0..3].
  //  commsDefault(config, I_ALL, I_NONE); // get a default configuration.
  //  commsSave(config); // write to EEPROM.
  // end boostrap

  myNodeID = commsStart(); // assume that EEPROM has been used to correctly set values
  radio.setHighPower(); // for HW boards.
  radio.promiscuous(true); // so broadcasts are received.
  
  if ( myNodeID == 0 ) {
    Serial << F("Unable to recover RFM settings!") << endl;
    while (1);
  }
  Serial << F("Radio startup complete. Node: ") << myNodeID << endl;

  // get last configuration
  commsLoad(config);
  commsPrint(config, myNodeID);
  // set flame cooldown from config
  flameCoolDownTime.interval(config.flameCoolDownTime);

  // initialize instructions (i.e. none).
  commsDefault(inst);
  performInstruction();

}


void loop() {
  // put your main code here, to run repeatedly:

  // SAFETY: do not move this code after any other code.
  if ( flameOnTime.check() ) { // time to turn the flame off
    flameOff();
  }

  // check for comms traffic
  if ( radio.receiveDone() ) {
    // process it.
    if ( radio.DATALEN == sizeof(inst) ) {
      // save instruction for lights/flame
      inst = *(towerInstruction*)radio.DATA;
      // do it.
      performInstruction();
      // note network activity
      networkTimeout.reset();
      doTestPatterns = false;
      Serial << F("i") << endl;
    }
    else if ( radio.DATALEN == sizeof(config) ) {
      // configuration for tower
      Serial << F("Configuration from Console.") << endl;

      // check to see if the instructions have changed?
      if ( memcmp((void*)(&config), (void*)radio.DATA, sizeof(config)) != 0 ) {
        // yes, so grab it.
        config = *(towerConfiguration*)radio.DATA;
        // save it
        commsSave(config);
      }

      // show it
      commsPrint(config, myNodeID);

      // set flame cooldown from config
      flameCoolDownTime.interval(config.flameCoolDownTime);

      // note we're configured
      amConfigured = true;
    }
    else if ( radio.DATALEN == sizeof(inst) + 1 ) {
      // ping received.
      Serial << F("p");
    }
  } 
  
  if ( networkTimeout.check() ) { // comms are quiet
    doTestPatterns = true;
  }

  if( doTestPatterns ) {
    if( amConfigured ) updateNetworkTestPattern(); // if we need to update the test pattern, do so.
    else updateSoloTestPattern(); // if we need to update the test pattern, do so.
  }

  // check to see if we need to request configuration information from Console
//  if ( notConfigured && joinInterval.check() ) {
//    Serial << F("Join request.") << endl;
//    boolean toss = commsSend(config, consoleNodeID, 0); // ask for a configuration, no ACK request
//    joinInterval.interval(random(250, 500)); // important that each tower has its own timing to prevent Tx collision.
//    joinInterval.reset();
//  }

}

// generate a random number on the interval [a, b] with mode c.
unsigned long trandom(int a, int c, int b) {
  // using a triangular pdf
  // http://en.wikipedia.org/wiki/Triangular_distribution#Generating_Triangular-distributed_random_variates

  float cut = (c - a) / (b - a);

  float u = random(0, 101) / 100;

  if ( u < cut ) return ( a + sqrt( u * (b - a) * (c - a) ) );
  else return ( b - sqrt( (1 - u) * (b - a) * (b - c) ) );

}

void updateSoloTestPattern() {

  // pin locations
  static int pins[3] = {LED_R, LED_G, LED_B};

  // gotta start somewhere
  static byte currInd = N_COLORS;
  // track the time
  static Metro onTime(SOLO_TEST_PATTERN_UPDATE);

  if ( onTime.check() ) { // enter a new cycle
    // set to zero.
    commsDefault(inst);

    currInd = ++currInd % 4; // select the next color
    Serial << F("Tower Test Pattern.  Color: ") << currInd << endl;;

    inst.lightLevel[currInd] = 255;
    performInstruction();
    
    // reset the interval update to add some randomness.
    onTime.interval(trandom(SOLO_TEST_PATTERN_UPDATE/2, SOLO_TEST_PATTERN_UPDATE, SOLO_TEST_PATTERN_UPDATE*2));
    onTime.reset();
  }

}

// update lighting on an interval when there's nobody playing
void updateNetworkTestPattern() {
  // only update on an interval, or we're spamming the comms and using up a lot of CPU power with float operations
  static Metro boredUpdateInterval(NETWORK_TEST_PATTERN_UPDATE);
  if ( !boredUpdateInterval.check() ) return;
  boredUpdateInterval.reset(); // for next loop

  // set to zero.
  commsDefault(inst);

  // use a breathing function.
  float ts = millis() / 1000.0 / 2.0 * PI ;
  // put the colors out of phase, and turn off fire.
  inst.lightLevel[I_RED] = (exp(sin(ts + 0.00 * 2.0 * PI)) - 0.36787944) * 108.0;
  inst.lightLevel[I_GRN] = (exp(sin(ts + 0.25 * 2.0 * PI)) - 0.36787944) * 108.0;
  inst.lightLevel[I_BLU] = (exp(sin(ts + 0.50 * 2.0 * PI)) - 0.36787944) * 108.0;
  inst.lightLevel[I_YEL] = (exp(sin(ts + 0.75 * 2.0 * PI)) - 0.36787944) * 108.0;

  // do the instructions
  performInstruction();
}

void performInstruction() {
  // process inst through the filter of config

  // colors can be set independently.
  byte maxRed = 0;
  byte maxGrn = 0;
  byte maxBlu = 0;
  // check the settings
  if ( config.colorIndex == I_RED || config.colorIndex == I_ALL) {
    maxRed = max(maxRed, inst.lightLevel[I_RED]);
  }
  if ( config.colorIndex == I_GRN || config.colorIndex == I_ALL) {
    maxGrn = max(maxGrn, inst.lightLevel[I_GRN]);
  }
  if ( config.colorIndex == I_BLU || config.colorIndex == I_ALL) {
    maxBlu = max(maxBlu, inst.lightLevel[I_BLU]);
  }
  if ( config.colorIndex == I_YEL || config.colorIndex == I_ALL) {
    // get yellow by portion of red and green
    // NOTE: integer math used for speed.  floats are slow.
    maxRed = max(maxRed, inst.lightLevel[I_YEL]);
    maxGrn = max(maxGrn, (inst.lightLevel[I_YEL] * greenYellowFraction) / 100);
  }
  // set them, apping a final scaling to keep brightness roughly equivalent.
  lightSet(LED_R, map(maxRed, 0, 255, 0, RED_MAX));
  lightSet(LED_G, map(maxGrn, 0, 255, 0, GRN_MAX));
  lightSet(LED_B, map(maxBlu, 0, 255, 0, BLU_MAX));

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
  
  //  Serial << "setting pin " << lightPin << " to " << lightLevel << endl;
  if ( lightLevel <= 0 ) { // special case for fireLevel==0 i.e. "none"
    digitalWrite(lightPin, LOW); // also, faster than analogWrite
  } else if ( lightLevel >= 255 ) { // special case for fireLevel==255 i.e. "full"
    digitalWrite(lightPin, HIGH); // also, faster than analogWrite
  } else {
    // remap to scootch up to minimum before flicker.
    analogWrite(lightPin, max(MIN_PWM, lightLevel)); // analogWrite for everything between
  }
}

void flameOn(int fireLevel) {

  // special case for fireLevel==0 i.e. "none"
  if ( fireLevel == 0 ) return;

  // only if the cooldown timer is ready
  if ( flameCoolDownTime.check() ) {

    unsigned long flameTime = constrain( // constrain may be overkill, but map doesn't guarantee constraints
                                map(fireLevel, 1, 255, config.minFireTime, config.maxFireTime),
                                config.minFireTime, config.maxFireTime);

    // SAFETY: never, ever, ever set this pin high without resetting the flameOnTime timer, unless you
    // have some other way of turning it back off.

    // To put a Fine Point on it: this simple line will unleash 100,000 BTU.  Better take care.
    digitalWrite(FLAME, LOW); // This line should appear exactly once in a sketch, and be swaddled in all manner of caution.
    flameOnTime.interval(flameTime);
    flameOnTime.reset();
    
    Serial << F("Flame on!") << endl;

    // start cooldown counter
    flameCoolDownTime.reset();
  }
}

void flameOff() {
  digitalWrite(FLAME, HIGH);
}

void otherOn() {
  digitalWrite(OTHER, LOW);
}

void otherOff() {
  digitalWrite(OTHER, HIGH);
}
