#include "Sensor.h"

Bounce modeEnable = Bounce(MODE_ENABLE_PIN, SENSOR_DEBOUCE_TIME);
Bounce fireEnable = Bounce(FIRE_ENABLE_PIN, SENSOR_DEBOUCE_TIME);

#define ARMED_TRACK 900
#define DISARMED_TRACK 901


void Sensor::begin() { // remote control
  Serial << "Sensor: startup." << endl;

  pinMode(MODE_ENABLE_PIN, INPUT_PULLUP);
  pinMode(FIRE_ENABLE_PIN, INPUT_PULLUP);
}

// remote control.  There's a relay that will pull FIRE_ENABLE_PIN to LOW when pressed (enable fire).
// goes to HIGH when pressed again (disable fire).
// on the Towers, this same relay will physically prevent the accumulator solenoid from opening,
// so this is really a "FYI" for the Console.  We'll use that to make noise over the FM transmitter
// to let the Operator know what's up.
boolean Sensor::fireEnabled() {
  // track state
  static boolean fireMode = fireEnable.read() == FIRE_ENABLED;

  // is there a change in state?
  if ( fireEnable.update() ) {
    // fire enable/disable state has changed.
    Serial << "Fire status change" << endl;
    fireMode = fireEnable.read() == FIRE_ENABLED;

    if ( fireMode ) Serial << "Fire ENABLED!" << endl;
    else Serial << "Fire disabled!" << endl;

    // for tones
    sound.playTrack(fireMode ? ARMED_TRACK : DISARMED_TRACK);
  }

  // return mode
  return ( fireMode );
}

// remote control.  There's a relay that will pull MODE_ENABLE_PIN to LOW when pressed.
// goes to HIGH when pressed again.  We use this flip-flop to change modes
boolean Sensor::modeChange() {

  // is there a change in state?
  if ( modeEnable.update() ) {
    Serial << "Mode enable pin change!" << endl;
    return true;
  }
  else {
    return false;
  }

}

Sensor sensor;


