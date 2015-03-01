// Compile for Uno.

// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
#include <Streaming.h> // <<-style printing
#include <Bounce.h> // buttons
#include <Metro.h> // timers
#include <EEPROM.h> // saving and loading radio settings
#include <avr/eeprom.h> // eeprom block r/w
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

// this is where the lights and fire instructions from Console are placed
towerInstruction inst;
// structure stores how this Tower acts on those towerInstructions
towerConfiguration config;

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

// when we're out of the network, update the lights on this interval (approx)
#define SOLO_TEST_PATTERN_UPDATE 3000UL // ms

// when we're in the network and idle, update the lights on this interval
#define NETWORK_TEST_PATTERN_UPDATE 20UL // ms

// pin locations for outputs
#define LED_R 3 // the PWM pin which drives the red LED
#define LED_G 5 // the PWM pin which drives the green LED
#define LED_B 6 // the PWM pin which drives the blue LED
#define LED_W 9 // the PWM pin which drives the white LED (not connected)
#define FLAME 7 // relay for solenoid
#define OTHER 8 // relay (not connected)

// radio pins
#define D_CS_PIN 10 // default SS pin for RFM module

// remote control
#define RESET_PIN A0
#define GAME_ENABLE_PIN A1
#define DEBOUNCE_TIME 100UL // need a long debounce b/c electrical noise from solenoid.
Bounce systemReset = Bounce(RESET_PIN, DEBOUNCE_TIME);
Bounce gameEnable = Bounce(GAME_ENABLE_PIN, DEBOUNCE_TIME);
boolean gameEnableFlag = true;
boolean systemResetFlag = false;

// luminosity is not equal among LEDs
#define RED_MAX 255 // entry 255 in the gamma lookup
#define GRN_MAX 220 // entry 242 in the gamma lookup
#define BLU_MAX 210 // entry 238 in the gamma lookup

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(A3)); // or some other unconected pin

  // see: http://jeelabs.org/2011/11/09/fixing-the-arduinos-pwm-2/
  bitSet(TCCR1B, WGM12); // puts Timer1 in Fast PWM mode to match Timer0.

  Serial << F("Configuring output pins.") << endl;
  flameOff(); pinMode(FLAME, OUTPUT); // order is important.  set then output.
  otherOff(); pinMode(OTHER, OUTPUT); // order is important.  set then output.
  digitalWrite(LED_R, LOW); pinMode(LED_R, OUTPUT);
  digitalWrite(LED_G, LOW); pinMode(LED_G, OUTPUT);
  digitalWrite(LED_B, LOW); pinMode(LED_B, OUTPUT);

  Serial << F("Configuring input pins.") << endl;
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(GAME_ENABLE_PIN, INPUT_PULLUP);

  Serial << "System Reset: " << systemReseted() << endl;
  Serial << "Game Enable: " << gameEnabled() << endl;

  myNodeID = networkStart(); // assume that EEPROM has been used to correctly set values

  // get last configuration
  configLoad(config);
  configPrint(config, myNodeID);

  // set flame cooldown from config
  flameCoolDownTime.interval(config.flameCoolDownTime);

  // initialize with lighting on.
  whiteTestPattern();

}

void loop() {
  // SAFETY: do not move this code after any other code.
  if ( flameOnTime.check() ) { // time to turn the flame off
    flameOff();
  }
  
  // check system enable state
  if ( systemReset.update() ) { // reset state change
    Serial << F("Reset state change.  State: ");
    systemResetFlag = systemReseted();
    Serial << systemResetFlag << endl;
  }
  
  // check system enable state
  if ( gameEnable.update() ) { // gameplay enable state change
    Serial << F("Gameplay state change.  State: ");
    gameEnableFlag = gameEnabled();
    Serial << gameEnableFlag << endl;
  }

  // check for comms traffic
  if ( radio.receiveDone() ) {
    // process it.
    if ( radio.DATALEN == sizeof(inst) ) {
      // check to see if the instructions have changed?
      if ( memcmp((void*)(&inst), (void*)radio.DATA, sizeof(inst)) != 0 ) {
        // save instruction for lights/flame
        inst = *(towerInstruction*)radio.DATA;
        // do it.
        performInstruction();

        // note network activity
        networkTimeout.reset();
        networkTimedOut = false;

        Serial << F("I") << endl;
      } else {
        Serial << F("i") << endl;
      }
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
      Serial << F("p");
    }
  }

  if ( networkTimeout.check() ) {
    Serial << F("Network timeout.") << endl;
    networkTimedOut = true;
  }

  if ( systemResetFlag ) {
    // if the reset pin is held, turn to red lighting
//    redTestPattern();
//    delay(500);
    whiteTestPattern();
//    delay(500);
  } else if ( networkTimedOut ) {
    // comms are quiet, so some test patterns are appropriate
    if ( !gameEnableFlag ) {
      // if the gameplay pin is held, use white lighting after comms timeout
      whiteTestPattern();
    } else if ( amConfigured ) {
      // if we've gotten configuration from Console and comms timeout, cycle the lights
      inNetworkTestPattern(); // breathing pattern, indicates configuration received from Console
    } else {
      // if we've not gotten configuration from Console and comms timeout, flip the lights
      soloTestPattern();
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

boolean gameEnabled() {
  // with EMI, be very sure.
  Metro readTime(DEBOUNCE_TIME);
  readTime.reset();
  while( !readTime.check() ) gameEnable.update();

  // at system power up, relay is open, meaning pin will read HIGH.
  return ( gameEnable.read() == HIGH );
}

void whiteTestPattern() {
  instClear(inst);

  inst.lightLevel[I_RED] = RED_MAX;
  inst.lightLevel[I_GRN] = GRN_MAX;
  inst.lightLevel[I_BLU] = BLU_MAX;

  performInstruction();
}

void redTestPattern() {
  instClear(inst);

  inst.lightLevel[I_RED] = RED_MAX;

  performInstruction();
}

// generate a random number on the interval [a, b] with mode c.
long trandom(long a, long c, long b) {
  // using a triangular pdf
  // http://en.wikipedia.org/wiki/Triangular_distribution#Generating_Triangular-distributed_random_variates

  float cut = (c - a) / (b - a);

  float u = random(0, 101) / 100;

  if ( u < cut ) return ( a + sqrt( u * (b - a) * (c - a) ) );
  else return ( b - sqrt( (1 - u) * (b - a) * (b - c) ) );

}

void soloTestPattern() {
  // gotta start somewhere
  static byte currInd = random(0, N_COLORS);
  // track the time
  static Metro onTime(SOLO_TEST_PATTERN_UPDATE);

  if ( onTime.check() ) { // enter a new cycle
    // set to zero.
    instClear(inst);

    currInd = random(0, N_COLORS); // select the next color
    Serial << F("Test pattern (not configured).  Color: ") << currInd << endl;;

    inst.lightLevel[currInd] = 255;
    performInstruction();

    // reset the interval update to add some randomness.
    onTime.interval(random(SOLO_TEST_PATTERN_UPDATE / 2, SOLO_TEST_PATTERN_UPDATE / 2 + 1));
    onTime.reset();
  }

}

// update lighting on an interval when there's nobody playing
void inNetworkTestPattern() {
  static Metro boredUpdateInterval(NETWORK_TEST_PATTERN_UPDATE);
  // only update on an interval, or we're spamming the comms and using up a lot of CPU power with float operations
  if ( !boredUpdateInterval.check() ) return;
  boredUpdateInterval.reset(); // for next loop

  // set to zero.
  instClear(inst);

  // track the time
  static unsigned long cycleTime = 1000;
  static unsigned long tic = millis();
  static Metro cycleTimer(0);
  static byte currInd = random(0, N_COLORS);

  if ( cycleTimer.check() ) { // enter a new cycle
    currInd = random(0, N_COLORS);
    cycleTime = random(3000, 10001);
    Serial << F("Test pattern (configured).  Color: ") << currInd << F(" cycle time: ") << cycleTime << endl;
    cycleTimer.interval(cycleTime);
    cycleTimer.reset();
    tic = millis();
  }

  // use a breathing function.
  //  float ts = millis() / 1000.0 / 2.0 * PI ;
  float ts = float(millis() - tic) / float(cycleTime) * 2.0 * PI ;
  // start 3/4 phase out, at zero
  inst.lightLevel[currInd] = (exp(sin(ts + 0.75 * 2.0 * PI)) - 0.36787944) * 108.0;

  //  Serial << currInd << ": " << ts << " " << inst.lightLevel[currInd] << endl;

  //  inst.lightLevel[I_RED] = (exp(sin(ts + 0.00 * 2.0 * PI)) - 0.36787944) * 108.0;
  //  inst.lightLevel[I_GRN] = (exp(sin(ts + 0.25 * 2.0 * PI)) - 0.36787944) * 108.0;
  //  inst.lightLevel[I_BLU] = (exp(sin(ts + 0.50 * 2.0 * PI)) - 0.36787944) * 108.0;
  //  inst.lightLevel[I_YEL] = (exp(sin(ts + 0.75 * 2.0 * PI)) - 0.36787944) * 108.0;

  // do the instructions
  performInstruction();
}

void performInstruction() {
  // process inst through the filter of config

  // for yellow on a RGB strip, we need to active red and green in some proportion.
  // from testing, the red should be 74% and green 26%.
  // So, if run the red strip at the luminance requested
  // for yellow, then the green strip needs to be 0.35 = the ratio 0.26/0.74.
  const byte greenYellowFraction = 35;

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

// from: https://learn.adafruit.com/led-tricks-gamma-correction/the-quick-fix
#define gC(a) (pgm_read_word_near(gamma + a)) // use lookup table.
const uint8_t PROGMEM gamma[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

// minimum value on the LEDs that don't flicker
#define MIN_PWM 3

void lightSet(int lightPin, byte lightLevel) {

  byte cLightLevel = gC(lightLevel); // gamma correction applied by lookup table

  //  Serial << "setting pin " << lightPin << " to " << lightLevel << endl;
  if ( cLightLevel <= 0 ) { // special case for fireLevel==0 i.e. "none"
    digitalWrite(lightPin, LOW); // also, faster than analogWrite
  } else if ( cLightLevel >= 255 ) { // special case for fireLevel==255 i.e. "full"
    digitalWrite(lightPin, HIGH); // also, faster than analogWrite
  } else {
    // remap to scootch up to minimum before flicker.
    //    analogWrite(lightPin, max(MIN_PWM, lightLevel)); // analogWrite for everything between
    analogWrite(lightPin, cLightLevel); // analogWrite for everything between
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

    // set flame cooldown from config
    flameCoolDownTime.interval(config.flameCoolDownTime);

    // start cooldown counter
    flameCoolDownTime.reset();

    Serial << F("Flame on! ") << fireLevel << F(" mapped [") << config.minFireTime << F(",") << config.maxFireTime << F("]");
    Serial << F(" -> ") << flameTime << F("ms.  Cooldown: ") << config.flameCoolDownTime << F("ms.") << endl;
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

// starts the radio
byte networkStart() {

  Serial << F("Tower: Reading radio settings from EEPROM.") << endl;

  // EEPROM location for radio settings.
  const byte radioConfigLocation = 42;

  // try to recover settings from EEPROM
  byte offset = 0;
  byte nodeID = EEPROM.read(radioConfigLocation + (offset++));
  byte groupID = EEPROM.read(radioConfigLocation + (offset++));
  byte band = EEPROM.read(radioConfigLocation + (offset++));
  byte csPin = EEPROM.read(radioConfigLocation + (offset++));

  if ( groupID != D_GROUP_ID ) {
    Serial << F("Tower: EEPROM not configured.  Doing so.") << endl;
    // then EEPROM isn't configured correctly.
    offset = 0;
    EEPROM.write(radioConfigLocation + (offset++), towerNodeID[0]);
    //    EEPROM.write(radioConfigLocation + (offset++), towerNodeID[1]);
    //    EEPROM.write(radioConfigLocation + (offset++), towerNodeID[2]);
    //    EEPROM.write(radioConfigLocation + (offset++), towerNodeID[3]);

    EEPROM.write(radioConfigLocation + (offset++), D_GROUP_ID);
    EEPROM.write(radioConfigLocation + (offset++), RF69_915MHZ);
    EEPROM.write(radioConfigLocation + (offset++), D_CS_PIN);

    return ( networkStart() ); // go again after EEPROM save

  } else {
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
  Serial << F("Color(") << config.colorIndex << F(") Fire(") << config.fireIndex << F(") ");
  Serial << F("Flame min(") << config.minFireTime << F(") max(") << config.maxFireTime << F(").") << endl;
}
// clears all of the instructions 
void instClear(towerInstruction & inst) {
  for (int i = 0; i < N_COLORS; i++ ) {
    inst.lightLevel[i] = 0;
    inst.fireLevel[i] = 0;
  }
}
