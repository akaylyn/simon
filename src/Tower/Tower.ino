// Compile for Uno.

// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Bounce.h> // buttons
#include <Metro.h> // timers
#include <EEPROM.h> // saving and loading radio settings
#include <avr/eeprom.h> // eeprom block r/w
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module
#include <RGBlink.h> // control LEDs
#include <Timer.h> // control air injector
#include <IRremote.h> // control IR Rx lighting

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

// this is where the lights and fire instructions from Console are placed
towerInstruction inst;
// structure stores how this Tower acts on those towerInstructions
towerConfiguration config;
// structure tells us to switch modes.
modeSwitchInstruction modeSwitchInstr;

// Need an instance of the Radio Module
RFM69 radio;
// store my NODEID
byte myNodeID;
// keep track of activity to note network drop out
#define D_NETWORK_TIMOUT 10000UL
Metro networkTimeout(D_NETWORK_TIMOUT);

boolean amConfigured = false; // ask for config once after startup.
boolean networkTimedOut = true; // flip this false with traffic.

// track how long we need to have the flame on for.
Metro flameOnTime(100UL);
// but only fire this often.
Metro flameCoolDownTime(1000UL);

// spend this much time on a cycle of light updates when idle
#define TEST_PATTERN_PERIOD 10000UL // ms

// pin locations for outputs
#define LED_R 6 // the PWM pin which drives the red LED
#define LED_G 5 // the PWM pin which drives the green LED
#define LED_B 9 // the PWM pin which drives the blue LED
#define LED_IR 3 // the PWM pin which drives the IR floodlight
#define FLAME 7 // relay for flame effect solenoid
#define AIR 8 // relay for air solenoid

// radio pins
#define D_CS_PIN 10 // default SS pin for RFM module

// remote control
#define RESET_PIN A0
#define MODE_SWITCH_ENABLE_PIN A1
#define DEBOUNCE_TIME 100UL // need a long debounce b/c electrical noise from solenoid.
Bounce systemReset = Bounce(RESET_PIN, DEBOUNCE_TIME);
int mode = 0;
boolean systemResetFlag = false;

// RGB lighting tied together
LED light(LED_R, LED_G, LED_B);
// different lighting modes available.
#define SOLID 0
#define BLINK 1
#define FADE 2
#define TEST 3
#define FLASH 4

// Timer control for air solenoid
Timer air;
const unsigned long shortAirTime = 50UL;
const unsigned long longAirTime = 100UL;

// HSB model 411: http://www.tomjewett.com/colors/hsb.html

// RGBY HSB already defined.  Add some more:
const HSB dimWhite = {
  0, 0, 196};
const HSB cyan = {
  hstep * 3, 255, 255};
const HSB magenta = {
  hstep * 5, 255, 255};
const HSB white = {
  0, 0, 255}; // at 0 saturation, hue is meaningless
const HSB black = {
  0, 0, 0}; // off, essentially

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

// force overwrite of EEPROM.  useful for bootstrapping new Moteinos.
#define WRITE_EEPROM_NOW false

class Flood {
  public:
    void begin();
    
    void setBright(byte level);
    void setColor(byte color);
  
    void off();
    void on();
    
    void send(unsigned long data);
    void sendMultiple(unsigned long data, byte repeats);
    
    IRsend irsend;
    
  protected:
    int currentColor;
    int currentBright;    
    boolean isOn;
};

Flood flood;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(A3)); // or some other unconected pin

  // see: http://jeelabs.org/2011/11/09/fixing-the-arduinos-pwm-2/
  bitSet(TCCR1B, WGM12); // puts Timer1 in Fast PWM mode to match Timer0.

  Serial << F("Configuring output pins.") << endl;
  flameOff(); 
  pinMode(FLAME, OUTPUT); // order is important.  set then output.
  airOff(); 
  pinMode(AIR, OUTPUT); // order is important.  set then output.
  digitalWrite(LED_R, LOW); 
  pinMode(LED_R, OUTPUT);
  digitalWrite(LED_G, LOW); 
  pinMode(LED_G, OUTPUT);
  digitalWrite(LED_B, LOW); 
  pinMode(LED_B, OUTPUT);

  Serial << F("Configuring input pins.") << endl;
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(MODE_SWITCH_ENABLE_PIN, INPUT_PULLUP);

  Serial << "System Reset: " << systemReseted() << endl;
  Serial << "In mode: " << mode << endl;

  myNodeID = networkStart(WRITE_EEPROM_NOW); // assume that EEPROM has been used to correctly set values

  // get last configuration
  configLoad(config);
  configPrint(config, myNodeID);

  // start with a HSV test
  light.setMode(SOLID);
  
  // start the IR control of the floodlight
  flood.begin();
}

void loop() {
  // SAFETY: do not move this code after any other code.
  if ( flameOnTime.check() ) { // time to turn the flame off
    flameOff();
  }
  
  // check to see if we need to mess with the air.
  air.update();

  // call the leds update cycle for non-blocking fading
  light.update();

  // check system enable state
  if ( systemReset.update() ) { // reset state change
    systemResetFlag = systemReseted();
    Serial << F("Reset state change.  State: ");
    if( systemResetFlag ) Serial << F("reset.");
    else Serial << F("normal."); 
    Serial << endl;

    // reset pin is held
    if( systemResetFlag ) {
      light.setMode(BLINK);
      resetTestPattern();
    } 
    else {
      light.setMode(SOLID);
    }
  }

  // check for comms traffic
  if ( radio.receiveDone() ) {
    // process it.
    if ( radio.DATALEN == sizeof(inst) ) {
      // check to see if the instructions have changed?
      if ( memcmp((void*)(&inst), (void*)radio.DATA, sizeof(inst)) != 0 ) {
        // save instruction for lights/flame
        inst = *(towerInstruction*)radio.DATA;
        // do it.  filter instruction based on broadcast mode or directed to this Tower specifically.
        performInstruction(radio.TARGETID == myNodeID);

        Serial << F("I") << endl;
      } 
      else {
        Serial << F("i") << endl;
      }

      // note network activity
      networkTimeout.reset();
      networkTimedOut = false;
    } 
    else if ( radio.DATALEN == sizeof(config) ) {
      // configuration for tower
      Serial << F("Configuration from Console.") << endl;

      // check to see if the instructions have changed?
      if ( memcmp((void*)(&config), (void*)radio.DATA, sizeof(config)) != 0 ) {
        // yes, so grab it.
        config = *(towerConfiguration*)radio.DATA;
        // save it
        configSave(config);
      }

      // show it
      configPrint(config, myNodeID);

      // note we're configured
      amConfigured = true;
    } 
    else if ( radio.DATALEN == sizeof(inst) + 1 ) {
      // ping received.
      Serial << F("ping received.") << endl;
    }
    else if ( radio.DATALEN == sizeof(modeSwitchInstr)) {
      modeSwitchInstr = *(modeSwitchInstruction*)radio.DATA;

      mode = modeSwitchInstr.currentMode;
      Serial << F("Mode state change.  Going to mode: ") << mode << endl;

      // If we've gone to one of the test modes, display a color for 1.5 seconds.
      // This should be the same amount of time that the console is playing a 
      // sound, so the delay won't get us out of sync
      if(mode == 1) {
        light.setColor(red);
        myDelay(1500UL);
      }
      else if (mode == 2) {
        light.setColor(green);
        myDelay(1500UL);
      }
      else if (mode == 3) {
        light.setColor(blue);
        myDelay(1500UL);
      }
      else if (mode == 4) {
        light.setColor(yellow);
        myDelay(1500UL);
      }
    }
  }

  if ( networkTimeout.check() ) {
    Serial << F("Network timeout.") << endl;
    networkTimedOut = true;
  }
  if( systemResetFlag ) {
    resetTestPattern();
  }
  else if( networkTimedOut ) {
    // comms are quiet, so some test patterns are appropriate
    if(mode == 0) {
      // We're in gameplay mode, so use a fancy idle pattern
      // cycle the lights
      idleTestPattern();
    }
    else {
      // if we're not 
      disabledTestPattern();
    } 
  }

}

boolean systemReseted() {
  // with EMI, be very sure.
  Metro readTime(DEBOUNCE_TIME);
  readTime.reset();
  while( !readTime.check() ) systemReset.update();

  // at system power up, relay is open, meaning pin will read HIGH.
  return ( systemReset.read() == LOW );
}

void resetTestPattern() {
  light.setBlink(1000UL, 100UL);
  light.setColor(red);
}

void disabledTestPattern() {
  light.setColor(dimWhite); 
}

void idleTestPattern() {
  // track the period
  static unsigned long period = TEST_PATTERN_PERIOD;
  // where are we in the period
  unsigned long inPeriod = millis() % period;
  // which direction around the wheel are we going?
  static int direction = +1;
  // adjust the period and direction of wheel at zero
  if( inPeriod == 0 ) {
    period = random(TEST_PATTERN_PERIOD / 2, TEST_PATTERN_PERIOD * 2);
    direction *= -1;
    delay(1); // only want to trip this test once per period
    Serial << F("Test pattern period: ") << period << F(" direction: ") << direction << endl;    
  }

  // HSB to show
  HSB color;
  color.sat = 255;
  color.bri = 255;

  // pure red is appreciated as so much dimmer than blue or green, so we truncate the wheel to 
  // exclude mostly-red, and "reflect" from those hard limts.
  const int hueMin = 20;
  const int hueMax = 340;
  color.hue = direction == 1 ? map(inPeriod, 0, period, hueMin, hueMax) : map(inPeriod, 0, period, hueMax, hueMin);

  if( amConfigured ) {
    light.setMode(SOLID);
  } 
  else {
    light.setMode(BLINK);
    light.setBlink(1000UL, 25UL);
  }

  light.setColor(color);

}

void performInstruction(boolean isJustToMe) {
  // process inst through the filter of config

  // solid color
  light.setMode(SOLID);
  HSB color = black;

  byte floodColor = 11; // "off"
  
  // check the settings
  if ( isJustToMe || config.lightListen[I_RED] ) {
    color = mix(color, red, inst.lightLevel[I_RED]);
    // IR 
    if( inst.lightLevel[I_RED] >= 128 ) {
      floodColor = I_RED;
    }
  }
  if ( isJustToMe || config.lightListen[I_GRN] ) {
    color = mix(color, green, inst.lightLevel[I_GRN]);
    // IR 
    if( inst.lightLevel[I_GRN] > 128 ) {
      floodColor = I_GRN;
    }
  }
  if ( isJustToMe || config.lightListen[I_BLU] ) {
    color = mix(color, blue, inst.lightLevel[I_BLU]);
    // IR 
    if( inst.lightLevel[I_BLU] >= 128 ) {
      floodColor = I_BLU;
    }
  }
  if ( isJustToMe || config.lightListen[I_YEL] ) {
    color = mix(color, yellow, inst.lightLevel[I_YEL]);
    // IR 
    if( inst.lightLevel[I_YEL] >= 128 ) {
      floodColor = I_BLU;
    }
  }
  // flip the IR floodlight?
  if( floodColor != 11 ) {
    flood.setColor(floodColor);
  } else {
    flood.off();
  }
  // set the color on the tanks
  light.setColor(color);

  // fire is a little different, if we're "listening" to multiple fire channels.
  // assume we're not going to shoot fire.
  byte maxFireLevel = 0;
  // and then check for the longest requested fire length.
  if ( isJustToMe || config.fireListen[I_RED] ) {
    maxFireLevel = max(maxFireLevel, inst.fireLevel[I_RED]);
  }
  if ( isJustToMe || config.fireListen[I_GRN] ) {
    maxFireLevel = max(maxFireLevel, inst.fireLevel[I_GRN]);
  }
  if ( isJustToMe || config.fireListen[I_BLU] ) {
    maxFireLevel = max(maxFireLevel, inst.fireLevel[I_BLU]);
  }
  if ( isJustToMe || config.fireListen[I_YEL] ) {
    maxFireLevel = max(maxFireLevel, inst.fireLevel[I_YEL]);
  }
  // execute on the longest requested flame length.
  flameOn(maxFireLevel, inst.flameEffect);

}

void flameOn(int fireLevel, flameEffect_t effect) {

  // special case for fireLevel==0 i.e. "none"
  if ( fireLevel == 0 ) return;

  // only if the cooldown timer is ready
  if ( !flameCoolDownTime.check() ) return;

  unsigned long flameTime = constrain( // constrain may be overkill, but map doesn't guarantee constraints
          map(fireLevel, 1, 255, config.minFireTime, config.maxFireTime),
          config.minFireTime, config.maxFireTime
  );

  // SAFETY: never, ever, ever set this pin high without resetting the flameOnTime timer, unless you
  // have some other way of turning it back off.

  // To put a Fine Point on it: this simple line will unleash 100,000 BTU.  Better take care.
  digitalWrite(FLAME, LOW); // This line should appear exactly once in a sketch, and be swaddled in all manner of caution.
  flameOnTime.interval(flameTime);
  flameOnTime.reset();

  // set flame cooldown from config
  unsigned long cooldown = flameTime/config.flameCoolDownDivisor;
  flameCoolDownTime.interval(cooldown);

  // start cooldown counter
  flameCoolDownTime.reset();

  Serial << F("Flame on! ") << fireLevel << F(" mapped [") << config.minFireTime << F(",") << config.maxFireTime << F("]");
  Serial << F(" -> ") << flameTime << F("ms.  Cooldown: ") << cooldown << F("ms."); 
  Serial << F(" Effect: ") << effect << endl;
  
  // just making sure
  airOff();

  switch( effect ) {
    case FE_veryRich: // just straight propane (DEFAULT) "very rich"
      break; // well, that was easy. 

    case FE_kickStart:  // toss in some air at the beginning; can't start for 50ms
      for( unsigned long i = shortAirTime; i<(flameTime/3UL); i+=longAirTime)
        air.after(i, shortAirBurst); // load timers
      break;
    case FE_kickMiddle:  // toss in some air in the middle
      for( unsigned long i = max(shortAirTime,flameTime/3UL); i<(flameTime*2UL/3UL); i+=longAirTime)
        air.after(i, shortAirBurst); // load timers
      break;
    case FE_kickEnd:  // toss in some air at the end
      for( unsigned long i = max(shortAirTime,flameTime*2UL/3UL); i<flameTime; i+=longAirTime)
        air.after(i, shortAirBurst); // load timers
      break;

    case FE_gatlingGun: // short bursts of air throughout
      air.every(longAirTime+shortAirTime, shortAirBurst);
      break;
    case FE_randomly:  // toss in some air in a random pattern
      for( unsigned long i = shortAirTime; i<flameTime; i+=random(shortAirTime*3UL,shortAirTime*10UL))
        air.after(i, shortAirBurst); // load timers
      break;
    case FE_veryLean: // as much air as as we can before getting "too lean"
      air.every(shortAirTime+shortAirTime, shortAirBurst); 
      break;

  }
}

// pulse the air on for shortAirTime
void shortAirBurst() {
//  Serial << "Air.   short air:" << millis() << endl;
  air.pulseImmediate(AIR, shortAirTime, LOW);
}

void flameOff() {
  digitalWrite(FLAME, HIGH);
  airOff();

//  Serial << F("Flame off! ") << endl;
}

void airOff() {
  digitalWrite(AIR, HIGH);
  // clean out the event timers.
  for( uint8_t i=0; i<MAX_NUMBER_OF_EVENTS; i++) air.stop(i);
}

// starts the radio
byte networkStart(boolean overWriteEEPROM) {

  Serial << F("Tower: Reading radio settings from EEPROM.") << endl;

  // EEPROM location for radio settings.
  const byte radioConfigLocation = 42;

  // try to recover settings from EEPROM
  byte offset = 0;
  byte nodeID = EEPROM.read(radioConfigLocation + (offset++));
  byte groupID = EEPROM.read(radioConfigLocation + (offset++));
  byte band = EEPROM.read(radioConfigLocation + (offset++));
  byte csPin = EEPROM.read(radioConfigLocation + (offset++));

  if ( groupID != D_GROUP_ID || overWriteEEPROM ) {
    Serial << F("Tower: EEPROM not configured.  Doing so.") << endl;
    // then EEPROM isn't configured correctly.
    offset = 0;
    //EEPROM.write(radioConfigLocation + (offset++), towerNodeID[0]);
    //EEPROM.write(radioConfigLocation + (offset++), towerNodeID[1]);
    //EEPROM.write(radioConfigLocation + (offset++), towerNodeID[2]);
    //EEPROM.write(radioConfigLocation + (offset++), towerNodeID[3]);

    EEPROM.write(radioConfigLocation + (offset++), D_GROUP_ID);
    EEPROM.write(radioConfigLocation + (offset++), RF69_915MHZ);
    EEPROM.write(radioConfigLocation + (offset++), D_CS_PIN);

    return ( networkStart(false) ); // go again after EEPROM save

  } 
  else {
    Serial << F("Tower: Startup RFM12b radio module. ");
    Serial << F(" NodeID: ") << nodeID;
    Serial << F(" GroupID: ") << groupID;
    Serial << F(" Band: ") << band;
    Serial << F(" csPin: ") << csPin;
    Serial << endl;

    radio.initialize(band, nodeID, groupID);
    radio.setHighPower(); // for HW boards.
    radio.promiscuous(true); // so broadcasts are received.

    Serial << F("Tower: RFM69HW radio module startup complete. ");
    return (nodeID);
  }

}

// saves configuration to EEPROM
void configSave(towerConfiguration &config) {
  Serial << F("Writing towerConfiguration to EEPROM.") << endl;
  eeprom_write_block((const void*)&config, (void*)towerConfigLocation, sizeof(config));
}
// reads configuration from EEPROM
void configLoad(towerConfiguration &config) {
  Serial << F("Reading towerConfiguration from EEPROM.") << endl;
  eeprom_read_block((void*)&config, (void*)towerConfigLocation, sizeof(config));
}
// and prints current configuration
void configPrint(towerConfiguration &config, byte nodeID) {
  Serial << F("Tower (") << nodeID << F(") config: ");
  Serial << F("Color(");
  for(byte i=0; i<N_COLORS; i++) Serial << config.lightListen[i] << F(" ");
  Serial << F(") Fire(");
  for(byte i=0; i<N_COLORS; i++) Serial << config.fireListen[i] << F(" ");
  Serial << F(") ");
  Serial << F("Flame min(") << config.minFireTime << F(") max(") << config.maxFireTime << F(").");
  Serial << F("Flame cooldown divisor: ") << config.flameCoolDownDivisor << endl;
}
// clears all of the instructions 
void instClear(towerInstruction & inst) {
  for (int i = 0; i < N_COLORS; i++ ) {
    inst.lightLevel[i] = 0;
    inst.fireLevel[i] = 0;
  }
}

// alternative to delay(), non-blocking
void myDelay(unsigned long delayTime) {
  Metro delayFor(delayTime);
  delayFor.interval(delayTime);
  delayFor.reset();
  
  while( ! delayFor.check() ) {
    // insert any mission-critical operations here
    if( flameOnTime.check() ) flameOff();
  }
}


void Flood::begin() {
  // shut it all the way down
//  flood.sendMultiple(K24_OFF, 4);

  // on
  this->on();
  
  // gotta start somewhere
  setColor(99);
  currentBright = 0;
  setBright(N_BRIGHT_STEPS);
  
  Serial << F("Flood: on.") << endl;
}

void Flood::on() {
  send(K24_ON);
  this->isOn = true;
}
void Flood::off() {
  send(K24_OFF);
  this->isOn = false;
}

void Flood::setColor(byte color) {
  // might need to turn it on.
  if( ! this->isOn ) {
    this->on();
  }
  
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
  
  Serial << F("Flood: bright: ") << level << endl;
  if( level > currentBright ) {
    sendMultiple(K24_UP, level - currentBright);
  } else if( level < currentBright) {
    sendMultiple(K24_DOWN, currentBright - level);
  }
  // track
  currentBright = level;
}

void Flood::send(unsigned long data) {
  Serial << F("Flood: send: ") << _HEX(data) << endl;
  // simple wrapper
  noInterrupts();
  irsend.sendNEC(data, 32);
  interrupts();
}

void Flood::sendMultiple(unsigned long data, byte repeats) {
  send(data); 
  for( byte i=1; i < repeats; i++ ) {
    delay(8); // need a 8ms interval before resend
    send(data); 
//    send(K24_REPEAT);
  }
}


