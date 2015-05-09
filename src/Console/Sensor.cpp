#include "Sensor.h"

Bounce modeEnable = Bounce(MODE_ENABLE_PIN, SENSOR_DEBOUCE_TIME);
Bounce fireEnable = Bounce(FIRE_ENABLE_PIN, SENSOR_DEBOUCE_TIME);


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
    extern Sound sound;

    // this could be replaced by asking Music to play an mp3 file.  For now, we'll just use the tone system.
    for ( int i = 0; i < 3; i++ ) {
      // TODO: need a "klaxon" track!!!
//      sound.playTone(I_RED, !fireMode); // if fireMode, we'll get a RED tone (high freq); false, we get a WRONG tone (low freq)
//      delay( 250UL ); // wait 0.25 seconds
//      sound.stop();
//      delay( 100UL ); // wait 0.1 seconds
    }
  }

  // return mode
  return ( fireMode );
}

// remote control.  There's a relay that will pull MODE_ENABLE_PIN to LOW when pressed.
// goes to HIGH when pressed again.  We use this flip-flop to change modes
boolean Sensor::modeEnabledHasChanged() {
  // track state
  static boolean gamePlayMode = modeEnable.read() == MODE_ENABLED;

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


