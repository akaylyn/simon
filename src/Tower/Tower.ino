// Compile for Uno.

// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
// sizes, indexing and comms common to Towers and Console
#include <EEPROM.h> // saving and loading radio settings
#include <RFM12B.h> // RFM12b radio transmitter module
#include <Simon_Comms.h> 

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
RFM12B radio;
// store my NODEID
byte myNodeID;
// keep track of activity to note network drop out
#define D_NETWORK_TIMOUT 10000UL
Metro networkTimeout(D_NETWORK_TIMOUT);
// let the console know we're here to join the party.
boolean notConfigured = true; // ask for config once after startup.
Metro joinInterval(0); // but don't spam the channel

// track how long we need to have the flame on for.
Metro flameOnTime(100UL);
// but only fire this often.
Metro flameCoolDownTime(2000UL);

// pin locations for outputs
#define LED_R 6 // the PWM pin which drives the red LED
#define LED_G 9 // the PWM pin which drives the green LED
#define LED_B 5 // the PWM pin which drives the blue LED
#define FLAME 4 // JeeNode Port 1, pin 2 (DIO)
#define OTHER A0 // JeeNode Port 1, pin 5 (AIO)
#define GPIO1 7 // JeeNode Port 4, pin 2 (DIO)
#define GPIO2 A3 // JeeNode Port 4, pin 5 (AIO)

// minimum value on the LEDs that don't flicker
#define MIN_PWM 3

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(0));

  // see: http://jeelabs.org/2011/11/09/fixing-the-arduinos-pwm-2/
  bitSet(TCCR1B, WGM12);

  Serial << F("Configuring output pins.") << endl;
  flameOff(); pinMode(FLAME, OUTPUT); // order is important.  set low, then output.
  otherOff(); pinMode(OTHER, OUTPUT); // order is important.  set low, then output.
  digitalWrite(LED_R, LOW); pinMode(LED_R, OUTPUT);
  digitalWrite(LED_G, LOW); pinMode(LED_G, OUTPUT);
  digitalWrite(LED_B, LOW); pinMode(LED_B, OUTPUT);

  // once, at hardware initialization, we need to bootstrap some settings to the EEPROM
  //  commsSave(consoleNodeID); // write to EEPROM. Select consoleNodeID.
  //  commsSave(towerNodeID[0]); // write to EEPROM. Select towerNodeID[0..3].
  //  commsSave(towerNodeID[1]); // write to EEPROM. Select towerNodeID[0..3].
  //  commsSave(towerNodeID[2]); // write to EEPROM. Select towerNodeID[0..3].
  //  commsSave(towerNodeID[3]); // write to EEPROM. Select towerNodeID[0..3].
  //  commsDefault(config, I_ALL, I_NONE); // get a default configuration.
  //  commsSave(config); // write to EEPROM.
  // end boostrap

  myNodeID = commsStart(); // assume that RF12Demo has been used to correctly set values
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
  if ( radio.ReceiveComplete() && radio.CRCPass() ) {
    // we have radio comms
    if ( radio.GetDataLen() == sizeof(inst) ) {
      // save instruction for lights/flame
      inst = *(towerInstruction*)radio.Data;
      // do it.
      performInstruction();
    }
    else if ( radio.GetDataLen() == sizeof(config) ) {
      // configuration for tower
      Serial << F("Configuration from Console.") << endl;

      // note that
      notConfigured = false;

      // check to see if the instructions have changed?
      if ( memcmp((void*)(&config), (void*)radio.Data, sizeof(config)) != 0 ) {
        // yes, so grab it.
        config = *(towerConfiguration*)radio.Data;
        // save it
        commsSave(config);
      }

      // show it
      commsPrint(config, myNodeID);

      // set flame cooldown from config
      flameCoolDownTime.interval(config.flameCoolDownTime);
    }
    else if ( radio.GetDataLen() == sizeof(inst) + 1 ) {
      // ping received.
      Serial << F("+");
    }

    // check for ACK request
    if ( radio.ACKRequested() )
      radio.SendACK();

    // note activity
    networkTimeout.reset();

  } else if ( networkTimeout.check() ) { // otherwise we're on our own... lonely!

    updateTestPattern(); // if we need to update the test pattern, do so.

  }

  // check to see if we need to request configuration information from Console
  if ( notConfigured && joinInterval.check() ) {
    Serial << F("Join request.") << endl;
    boolean toss = commsSend(config, consoleNodeID, 0); // ask for a configuration, no ACK request
    joinInterval.interval(random(250, 500)); // important that each tower has its own.
    joinInterval.reset();
  }

}

void updateTestPattern() {

  // pin locations
  static int pins[3] = {LED_R, LED_G, LED_B};

  // gotta start somewhere
  static byte currInd = N_COLORS;
  // track the time
  static Metro onTime(5000);

  if ( onTime.check() ) { // enter a new cycle
    lightSet(pins[currInd], 0); // off

    currInd = ++currInd % 3; // select the next color
    Serial << F("Tower Test Pattern.  Color: ") << currInd << endl;;

    lightSet(pins[currInd], 255);
    onTime.reset();
  }

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
  // set them
  lightSet(LED_R, maxRed);
  lightSet(LED_G, maxGrn);
  lightSet(LED_B, maxBlu);

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

    digitalWrite(FLAME, HIGH); // This line should appear exactly once in a sketch, and be swaddled in all manner of caution.
    Serial << F("Flame on!") << endl;

    flameOnTime.interval(flameTime);
    flameOnTime.reset();

    // start cooldown counter
    flameCoolDownTime.reset();

  }
}

void flameOff() {
  digitalWrite(FLAME, LOW);
}

void otherOn() {
  digitalWrite(OTHER, HIGH);
}

void otherOff() {
  digitalWrite(OTHER, LOW);
}
