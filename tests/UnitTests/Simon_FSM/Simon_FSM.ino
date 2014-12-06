#include <FiniteStateMachine.h>
#include <LED.h>
#include <Metro.h>
#include <Bounce.h>
#include <Streaming.h>
#include "Definitions.h"
#include "notes.h"
#include "rtttl.h"
#include "win_songs.h"

// sizes, indexing and comms between Towers and Console
#include <EEPROM.h> // saving and loading radio settings
#include <RFM12B.h> // RFM12b radio transmitter module
#include <Simon_Comms.h>

// communications: RFM12b
#include "Comms.h"
// this is where the lights and fire instructions to Towers are placed
towerInstruction inst;
// from Simon_Comms.h:
// typedef struct {
//	byte lightLevel[N_COLORS]; // 0..255.  maps to analogWrite->light level
//	byte fireLevel[N_COLORS]; // 0..255.  maps to timer->fire duration
// } towerInstruction;

// timers are used extensively to trigger events
//
// during idle, do a fanfare of light, sound and fire
Metro kioskTimer(KIOSK_FANFARE_MAX);
// during play, the player can pause between button presses
Metro playerTimeout(PLAYER_TIMEOUT);
// how often do we relight Mr Sparky?
Metro sparkyInterval(SPARKY_RELIGHT_INTERVAL);
// how often do we trigger with motion?
Metro motionInterval(MOTION_INTERVAL);

// system inputs
// buttons
Bounce redButton = Bounce(RED_SWITCH, DEBOUNCE_TIME);
Bounce grnButton = Bounce(GRN_SWITCH, DEBOUNCE_TIME);
Bounce bluButton = Bounce(BLU_SWITCH, DEBOUNCE_TIME);
Bounce yelButton = Bounce(YEL_SWITCH, DEBOUNCE_TIME);
// PIR
Bounce PIR = Bounce(PIR_WIRE, DEBOUNCE_TIME);

// system outputs
// lights
LED redLight(RED_LIGHT);
LED grnLight(GRN_LIGHT);
LED bluLight(BLU_LIGHT);
LED yelLight(YEL_LIGHT);
// music
Rtttl musicPlayer;
// sound

// implemented Simon as a finite state machine
// this is the definitions of the states that our program uses
State idle = State(idleState);
State game = State(gameState);
State player = State(playerState);

// the state machine controls which of the states get attention and execution time
FSM simon = FSM(idle); //initialize state machine, start in state: idle

// track the game states
//
// the sequence length, [0, MAX_SEQUENCE_LENGTH-1]
int currentLength = 0;
// track the sequence.
char correctSequence[MAX_SEQUENCE_LENGTH];
// the count of corrent matches that the player has made.  <= currentLength
int correctLength = 0;

// when there's nobody playing, we're in this state.
void idleState() {
  if ( currentLength > 0 ) {
    // somebody played, so let's give them a show
    doWinnerFanfare();
    // reset timers
    kioskTimer.reset();
    motionInterval.reset();
  }
  // reset everything.
  correctLength = 0;
  currentLength = 0;
  correctSequence[0] = '\0';

  if ( anyButtonPressed() ) {
    Serial << "Idle.  going to Game." << endl;
    allLightsOff();
    quiet();
    while ( ! anyButtonChanged() );
    // let's play a game
    simon.transitionTo(game);
  } else if ( kioskTimer.check() ) {
    // let's do some light, sound and fire
    doKioskFanfare();
  } else if ( sparkyInterval.check() ) {
    // let's relight the pilot
    relightPilot();
  } else {
    // let's rotate the lighting slightly
    doKioskBored();
  }

  // turn off the SSR
  flameOff();
  
}

// when the player has played, or at the start of the game, we're in this state
void gameState() {
  // generate the next in sequence and add that to the sequence
  correctSequence[currentLength] = nextMove();
  correctSequence[currentLength + 1] = '\0'; // and null terminate it for printing purposes.
  // increment
  currentLength++;
  // play back sequence
  Serial << "Game: currentLength=" << currentLength << ". sequence=" << correctSequence << ". Back to Player." << endl;
  playSequence();
  // debug

  // reset timeout
  playerTimeout.reset();
  // reset correct length
  correctLength = 0;
  // give it back to the player
  simon.transitionTo(player);
}

// waiting for the player to press a button.
void playerState() {
  // check if they waited too long
  if ( playerTimeout.check() ) {
    Serial << "Player: timeout.  currentLength = " << currentLength << ".  Back to Idle." << endl;
    // if so, show the correct next button
    play(correctSequence[correctLength], false);
    // wait
    delay(2000);
    quiet();
    // and exit game play
    simon.transitionTo(idle);
  }

  // otherwise, wait for button press.
  if ( anyButtonPressed() ) {
    boolean correct = (grnButtonPressed() && correctSequence[correctLength] == 'G') ||
                      (redButtonPressed() && correctSequence[correctLength] == 'R') ||
                      (bluButtonPressed() && correctSequence[correctLength] == 'B') ||
                      (yelButtonPressed() && correctSequence[correctLength] == 'Y');

    // there's a total cheat.
    correct = correct || CHEATY_PANTS_MODE;
    
    // light and sound
    play( correctSequence[correctLength], correct );
    // hold it while we're mashing
    while ( ! anyButtonChanged() );

    // done
    quiet();
    allLightsOff();

    if ( correct ) {
      correctLength++;
      // reset timeout
      playerTimeout.reset();
      // keep going
    } else {
      Serial << "Player: incorrect.  currentLength = " << currentLength << ".  Back to Idle." << endl;
      // if so, show the correct next button
      play(correctSequence[correctLength], false);
      // wait
      delay(3000);
      quiet();
      // and exit game play
      // exit game play
      simon.transitionTo(idle);
    }
  }

  // check if they've got the sequence complete, or if there's just awesome
  if ( correctLength == currentLength || correctLength == (MAX_SEQUENCE_LENGTH - 1) ) {
    // nice.  pass it back to game
    Serial << "Player: correct.  currentLength = " << currentLength << ".  Back to Game." << endl;
    simon.transitionTo(game);
  }
}

void play(char color, boolean correctTone) {
  // lights and sound
  switch ( color ) {
    case 'G':
      grnLight.setValue(LED_ON);
      tone(SPEAKER_WIRE, correctTone ? GRN_TONE : WRONG_TONE);
      break;
    case 'R':
      redLight.setValue(LED_ON);
      tone(SPEAKER_WIRE, correctTone ? RED_TONE : WRONG_TONE);
      break;
    case 'B':
      bluLight.setValue(LED_ON);
      tone(SPEAKER_WIRE, correctTone ? BLU_TONE : WRONG_TONE);
      break;
    case 'Y':
      yelLight.setValue(LED_ON);
      tone(SPEAKER_WIRE, correctTone ? YEL_TONE : WRONG_TONE);
      break;
  }
}

void quiet() {
  noTone(SPEAKER_WIRE);
}

void playSequence() {
  // how long between each
  unsigned long pauseDuration = 50; // ms
  // how long to light and tone
  unsigned long playDuration = 420;
  if ( currentLength >= 6 ) playDuration = 320;
  if ( currentLength >= 14) playDuration = 220;

  // delay after a player's last move
  delay(playDuration);

  for (int i = 0; i < currentLength; i++) {
    // lights and sound
    play(correctSequence[i], true);
    // wait
    delay(playDuration);

    // done
    quiet();
    allLightsOff();

    // pause between next
    delay(pauseDuration);
  }
}

void allLightsOn() {
  grnLight.setValue(LED_ON);
  redLight.setValue(LED_ON);
  bluLight.setValue(LED_ON);
  yelLight.setValue(LED_ON);
}

void allLightsOff() {
  grnLight.setValue(LED_OFF);
  redLight.setValue(LED_OFF);
  bluLight.setValue(LED_OFF);
  yelLight.setValue(LED_OFF);
}

void doKioskBored() {
  // use a breathing function.
  float ts = millis() / 1000.0;
  // put the colors out of phase
  float redVal = (exp(sin(ts / 2.0 * PI + 0.00 * 2.0 * PI)) - 0.36787944) * 108.0;
  float grnVal = (exp(sin(ts / 2.0 * PI + 0.25 * 2.0 * PI)) - 0.36787944) * 108.0;
  float bluVal = (exp(sin(ts / 2.0 * PI + 0.50 * 2.0 * PI)) - 0.36787944) * 108.0;
  float yelVal = (exp(sin(ts / 2.0 * PI + 0.75 * 2.0 * PI)) - 0.36787944) * 108.0;

  //  Serial << redVal << " " << grnVal << " " << bluVal << " " << yelVal << endl;
  redLight.setValue(redVal);
  grnLight.setValue(grnVal);
  bluLight.setValue(bluVal);
  yelLight.setValue(yelVal);
  
  // patch in the comms
  if( inst.lightLevel[I_RED] != byte(redVal) ) {
    inst.lightLevel[I_RED] = redVal;
    towerSend();
  }
  if( inst.lightLevel[I_GRN] != byte(grnVal) ) {
    inst.lightLevel[I_GRN] = grnVal;
    towerSend();
  }
  if( inst.lightLevel[I_BLU] != byte(bluVal) ) {
    inst.lightLevel[I_BLU] = bluVal;
    towerSend();
  }
  if( inst.lightLevel[I_YEL] != byte(yelVal) ) {
    inst.lightLevel[I_YEL] = yelVal;
    towerSend();
  }

  // if there's motion we can detect, maybe make some fire
  if ( PIRTriggered() && motionInterval.check() ) {
    Serial << "Idle.  motion detected." << endl;
    int motionTriggerProb = getKioskActivity() / 3;
    if ( random(100) < motionTriggerProb ) {
      allLightsOn();
      doPoofs(random(MIN_POOF_COUNT, MAX_POOF_COUNT + 1));
      allLightsOff();
    }
    motionInterval.reset();
  }
}

void doKioskFanfare() {
  Serial << "! Kiosk Fanfare" << endl;

  // pick a song
  int minSong = 22;
  int maxSong = 121;
  int song = random(minSong, maxSong+1);
  Serial << " song: " << song << endl;
  musicPlayer.play_P(song_table[song], 0);  // tunez and fire.  yeah.

  // reset the timer
  delay(300); // be still, my electrical heart
  unsigned long kioskTimerInterval = map(getKioskActivity(), 0, 100, KIOSK_FANFARE_MIN, KIOSK_FANFARE_MAX);
  kioskTimer.interval(kioskTimerInterval);
  Serial << "Kiosk timer reset: " << kioskTimerInterval << endl;
  kioskTimer.reset();
}

void doWinnerFanfare() {
  Serial << "! Winner. Fanfare." << endl;

  // pick a song
  // 0-21 <5 sec
  // 22-121 <17 sec
  // 122-138 <45 sec
  int minSong = 0;
  int maxSong = 21;
  
  if ( currentLength >= LEVEL_1 ) {
    // start with a bang
    allLightsOn();
    doPoofs(map(currentLength, 1, MAX_SEQUENCE_LENGTH, MIN_POOF_COUNT, MAX_POOF_COUNT + 1));
    allLightsOff();

    minSong = 22;
    maxSong = 121; 
  }
  if( currentLength >= LEVEL_2 ) {
    minSong = 122;
    maxSong = NUM_SONGS-1;
  }

 int song = random(minSong, maxSong+1);

  // check correctLength relative to MAX_SEQUENCE_LENGTH, and pick a song in the list
//  int max_song = (NUM_SONGS - 1) * currentLength / MAX_SEQUENCE_LENGTH;
  // pick a song in a band around, assuming songs further in the sequence are longer = more fire
//  int song = constrain(random(max(0, max_song - 20), (max_song + 1)), 0, (NUM_SONGS - 1));
  Serial << "! Winner. currentLength: " << currentLength << " song: [" << minSong << "-" << maxSong << "]. song: " << song << endl;
  musicPlayer.play_P(song_table[song], 0);  // tunez and fire.  yeah.

  // special case: correctLength == (MAX_SEQUENCE_LENGTH-1)
  // in this case, spend the accumulator in a column of fire.  well done, Player 1.
  if ( currentLength == (MAX_SEQUENCE_LENGTH - 1) ) {
    Serial << "! Winner: special case.  Opening the FLAME!" << endl;
    allLightsOn();
    while ( getPressureLevel() > 10 ) flameOn();
    flameOff();
    allLightsOff();
    Serial << "! Winner. special case.  Closing the FLAME!" << endl;
  }
}

// .update() must be called periodically for debouncing.
boolean redButtonChanged() {
  return ( redButton.update() );
}
boolean grnButtonChanged() {
  return ( grnButton.update() );
}
boolean bluButtonChanged() {
  return ( bluButton.update() );
}
boolean yelButtonChanged() {
  return ( yelButton.update() );
}
boolean PIRChanged() {
  return ( PIR.update() );
}

boolean anyButtonChanged() {
  // update the buttons and return a state change as true.
  boolean redC = redButtonChanged();
  boolean grnC = grnButtonChanged();
  boolean bluC = bluButtonChanged();
  boolean yelC = yelButtonChanged();
  // actively avoiding short-circuit evaluation so that we get an .update() to all.
  return ( redC || grnC || bluC || yelC );
}

// the .update() is wrapped in here, too, but discarded.
boolean redButtonPressed() {
  boolean foo = redButtonChanged();
  return ( redButton.read() == PRESSED );
}
boolean grnButtonPressed() {
  boolean foo = grnButtonChanged();
  return ( grnButton.read() == PRESSED );
}
boolean bluButtonPressed() {
  boolean foo = bluButtonChanged();
  return ( bluButton.read() == PRESSED );
}
boolean yelButtonPressed() {
  boolean foo = yelButtonChanged();
  return ( yelButton.read() == PRESSED );
}
boolean PIRTriggered() {
  boolean foo = PIRChanged();
  return ( PIR.read() == PIR_TRIGGERED );
}

boolean anyButtonPressed() {
  // wrap the update in this, too.
  return ( redButtonPressed() ||
           grnButtonPressed() ||
           bluButtonPressed() ||
           yelButtonPressed() );
}


void setup() {
  // switches
  pinMode(RED_SWITCH, INPUT_PULLUP);
  pinMode(GRN_SWITCH, INPUT_PULLUP);
  pinMode(BLU_SWITCH, INPUT_PULLUP);
  pinMode(YEL_SWITCH, INPUT_PULLUP);

  // sound
  pinMode(SPEAKER_WIRE, OUTPUT);
  musicPlayer.begin(SPEAKER_WIRE);

  // button lights, panels, under console.  wire to N-channel MOSFET + and any GND.
  allLightsOff();

  // new pots attached to A1 and A2 and pulled to GND
  pinMode(POT1_WIRE, INPUT_PULLUP);
  pinMode(POT2_WIRE, INPUT_PULLUP);

  // IR
  pinMode(PIR_WIRE, INPUT_PULLUP);

  // ignitor
  pinMode(SPARKY_WIRE, OUTPUT);

  // accumulator
  pinMode(ACCUM_WIRE, OUTPUT);

  // pressure sensor
  pinMode(PRESSURE_WIRE, INPUT);

  // serial comms
  Serial.begin(DEBUG_RATE);

  // for random
  randomSeed(analogRead(A7));

  // use a quick delay to calibrate buttons
  playerTimeout.reset();
  while ( ! playerTimeout.check() ) anyButtonChanged();

  // start RFM12b radio
  if ( commsStart() != consoleNodeID  ) {
    Serial << F("Unable to start RFM as consoleNodeID.  Halting!") << endl;
    while (1);
  }
  
  // ping the network
  pingNetwork();
  // configure network
  configureNetwork();
  // update network
  sendConfiguration();
  // default instruction
  commsDefault(inst);
  // send it
  commsSend(inst);

}

void loop() {
  // call to poll the radio module for traffic, check in with the network
  towerComms();

  // update FSM
  simon.update();

//  currentLength = 24;
//  Serial << "*** " << currentLength << endl;
//  doWinnerFanfare();
}

// read and average
int analogReadDebounce(int pin) {
  unsigned long sum = 0;
  for (int i = 0; i < ANALOG_DEBOUNCE_COUNT; i++) {
    sum += analogRead(pin);
  }
  return ( sum / ANALOG_DEBOUNCE_COUNT );

}

// Read analog input, scaling [min_value, max_value] to [0,100]
int readAnalog(int pin, unsigned long min_value, unsigned long max_value) {
  // take a reading, debounced
  int reading = analogReadDebounce(pin);

  // map reading 0-100
  int scaled = map(constrain(reading, min_value, max_value), min_value, max_value, 0, 100);

  Serial << "pin: " << pin << " read: " << reading << ". scale: [" << min_value << "-" << max_value << "]. returning: " << scaled << endl;

  return ( scaled );
}

// kiosk activity knob
int getKioskActivity() {
  Serial << "~ Kiosk Activity Reading: ";
  return ( readAnalog(POT1_WIRE, POT_MIN, POT_MAX) );
}

// flame abundance setting
int getFlameSetting() {
  Serial << "~ Flame Prob Reading: ";
  return ( readAnalog(POT2_WIRE, POT_MIN, POT_MAX) );
}

// pressure level reading
int getPressureLevel() {
  Serial << "~ Pressure Reading: ";
  // sensor generates 0.5V at 0 psig and 5V at 100 psig
  return ( readAnalog(PRESSURE_WIRE, PRESSURE_MIN, PRESSURE_MAX) );
}

unsigned long flameOnTime(int a, int c, int b) {
  // using a triangular pdf
  // http://en.wikipedia.org/wiki/Triangular_distribution#Generating_Triangular-distributed_random_variates

  float cut = (c - a) / (b - a);

  float u = random(0, 101) / 100;

  if ( u < cut ) return ( a + sqrt( u * (b - a) * (c - a) ) );
  else return ( b - sqrt( (1 - u) * (b - a) * (b - c) ) );

}

void doPoofs(int poofCount) {
  Serial << "! Poofs. N:" << poofCount << endl;
  for (int i = 0; i < poofCount; i++) {
    flameOn();
    delay(flameOnTime(POOF_MIN, POOF_MODE, POOF_MAX));
    flameOff();
    delay(random(WAIT_MIN, WAIT_MAX));
  }
}

void relightPilot() {
  // turn off the accumulator release
  flameOff();

  // relight pilot
  Serial << "! Pilot relight." << endl;
  digitalWrite(SPARKY_WIRE, SPARKY_ON);
  delay(SPARKY_DURATION);
  digitalWrite(SPARKY_WIRE, SPARKY_ON);

  // reset relight interval
  sparkyInterval.reset();
}

void flameOff() {
  // turn on flame
  digitalWrite(ACCUM_WIRE, ACCUM_OFF);
}
void flameOn() {
  // turn off flame
  digitalWrite(ACCUM_WIRE, ACCUM_ON);
}


char nextMove() {
  int move = random(1, 4 + 1);
  // 1=GRN, 2=RED, 3=BLU, 4=YEL
  switch ( move ) {
    case 1: return ('G');
    case 2: return ('R');
    case 3: return ('B');
    case 4: return ('Y');
  }
}


