// The IDE requires all libraries to be #include’d in the main (.ino) file.  Clutter.
// included in several places.
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
// wiring connections
#include "Pinouts.h"
// capsense touch: MPR121 include I2C, MPR121 libraries.
#include <Wire.h>
#include <Adafruit_MPR121.h>
#include <Bounce.h> // manual buttons as well
#include <LED.h> // manual lights as well
#include "Touch.h"
#include "Music.h"
// communications: RFM12b
// sizes, indexing and comms between Towers and Console
#include <EEPROM.h> // saving and loading radio settings
#include <SPI.h>
#include <RFM12B.h> // RFM12b radio transmitter module
#include <Simon_Comms.h>
#include "Comms.h"
// game play
#include <FiniteStateMachine.h>
#include "Gameplay.h"

// this is where the lights and fire instructions to Towers are placed
towerInstruction inst;
// from Simon_Comms.h:
// typedef struct {
//	byte lightLevel[N_COLORS]; // 0..255.  maps to analogWrite->light level
//	byte fireLevel[N_COLORS]; // 0..255.  maps to timer->fire duration
// } towerInstruction;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(0));

  // switches
  configureManualButtons();
  configureManualLights();
  // and tone sounds from Mega
  configureManualSound();

  // start MPR121 touch
  //  touchStart();
  //  touchUnitTest(false); // no details

  // shouldn't need to run this once the memory is saved.
  //  commsSave(consoleNodeID); // write to EEPROM. Select consoleNodeID.
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


}

// main loop for the core.
void loop() {
  // update the FSM
  updateSimonFSM();

  // call to poll the radio module for traffic, check in with the network
  towerComms();
  
}

// replace with gameplay
boolean towerUpdate( ) {
  return ( updateTestPattern() );
}

// simple test pattern.
boolean updateTestPattern() {

  // total cycle time
  const unsigned long cycleLength = 5000; // ms.  2.5 second cycle

  // only send an update on this interval
  static Metro updateInterval(10); // ms updates, maximum.  could be longer than this.

  // return true if we need to update
  boolean needUpdate = false;
  // gotta start somewhere
  static byte currInd = N_COLORS - 1;
  // track the last position in a cycle
  static unsigned long ltc = 1;
  // track the last value setting
  static byte lastval = 0;
  // track the number of updates per cycle
  static int trackCount = 0;

  if ( updateInterval.check() ) {

    // we'll use the commsInstruction instrument to simulate received instructions
    // use a breathing function.

    // calculate where we are
    unsigned long tc = millis() % cycleLength; // 0..cycleLength

    // compare to the last tc
    if ( tc < ltc ) { // enter a new cycle
      commsDefault(inst, 0, 0); // reset instructions, making sure fire is off.
      manualLightSet(I_ALL, LED_OFF); // manual lights udpate
      currInd = ++currInd % N_COLORS; // select the next color
      needUpdate = true;
      Serial << F("Track count: ") << trackCount << endl;
      trackCount = 0;
      Serial << F("Console Test Pattern.  Color: ") << currInd << endl;;
    }

    // compute exp sine, cast to byte as that's what we're capable of
    byte val = (exp(sin(float(tc) / float(cycleLength) * 2.0 * PI + 0.75 * 2.0 * PI)) - 0.36787944) * 108.0;

    if ( val != lastval || needUpdate ) { // if the value changed, or if we've flipped to a new color
//      Serial << tc - ltc << F(",") << val << endl;
      lastval = val; // save current val
      inst.lightLevel[currInd] = val;
      trackCount++;
      needUpdate = true;
      manualLightSet(currInd, val); // manual lights udpate
      updateInterval.reset(); // reset for next loop
    }

    ltc = tc; // save current position
  }

  return ( needUpdate );

}


