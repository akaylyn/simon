#include "Button.h"

// buttons
Bounce redButton = Bounce(BUTTON_RED, BUTTON_DEBOUNCE_TIME);
Bounce grnButton = Bounce(BUTTON_GRN, BUTTON_DEBOUNCE_TIME);
Bounce bluButton = Bounce(BUTTON_BLU, BUTTON_DEBOUNCE_TIME);
Bounce yelButton = Bounce(BUTTON_YEL, BUTTON_DEBOUNCE_TIME);

// configures buttons at startup
boolean buttonStart() {
  pinMode(BUTTON_RED, INPUT_PULLUP);
  pinMode(BUTTON_GRN, INPUT_PULLUP);
  pinMode(BUTTON_BLU, INPUT_PULLUP);
  pinMode(BUTTON_YEL, INPUT_PULLUP);

  Serial << F("Button: startup.") << endl;
  return( true );
}

// returns true if any of the buttons have switched states.
boolean buttonAnyChanged() {
  // avoid short-circuit eval so that each button gets an update
  // for the debounce code.
  boolean redC = buttonChanged(I_RED);
  boolean grnC = buttonChanged(I_GRN);
  boolean bluC = buttonChanged(I_BLU);
  boolean yelC = buttonChanged(I_YEL);
  return ( redC || grnC || bluC || yelC );
}

// returns true if a specific button has changed
boolean buttonChanged(byte buttonIndex) {
  /* this function will be called very frequently, so this is where the capsense
     calls should be made and the debouncing routines should reside
  */
  // this function will be called after buttonChanged() asserts a change.
  switch ( buttonIndex ) {
    case I_RED:
      return (redButton.update());
      break;
    case I_GRN:
      return (grnButton.update());
      break;
    case I_BLU:
      return (bluButton.update());
      break;
    case I_YEL:
      return (yelButton.update());
      break;
    case I_ALL:
      return ( buttonAnyChanged() );
      break;
    default:
      Serial << F("Button: buttonChanged Error, case=") << buttonIndex << endl;

  }

}

// returns true if any of the buttons are pressed.
boolean buttonAnyPressed() {
  return (
           buttonPressed(I_RED) ||
           buttonPressed(I_GRN) ||
           buttonPressed(I_BLU) ||
           buttonPressed(I_YEL)
         );
}

// returns true if a specific button is pressed
boolean buttonPressed(byte buttonIndex) {
  // call the updater for debouncing first.
  boolean toss = buttonChanged( buttonIndex );
  
  // this function will be called after buttonChanged() asserts a change.
  switch ( buttonIndex ) {
    case I_RED:
      return (redButton.read() == PRESSED_BUTTON);
      break;
    case I_GRN:
      return (grnButton.read() == PRESSED_BUTTON);
      break;
    case I_BLU:
      return (bluButton.read() == PRESSED_BUTTON);
      break;
    case I_YEL:
      return (yelButton.read() == PRESSED_BUTTON);
      break;
    case I_ALL:
      return ( buttonAnyPressed() );
      break;
    default:
      Serial << F("Button: buttonPressed Error, case=") << buttonIndex << endl;
  }
}

// unit test for buttons
void buttonUnitTest() {
  while(1) {
    if( buttonPressed(I_RED) ) {
      Serial << F("Button: RED pressed.") << endl;
      while ( buttonAnyPressed() );
      Serial << F("Button: RED released.") << endl;
    }
    if( buttonPressed(I_GRN) ) {
      Serial << F("Button: GRN pressed.") << endl;
      while ( buttonAnyPressed() );
      Serial << F("Button: GRN released.") << endl;
    }
    if( buttonPressed(I_BLU) ) {
      Serial << F("Button: BLU pressed.") << endl;
      while ( buttonAnyPressed() );
      Serial << F("Button: BLU released.") << endl;
    }
    if( buttonPressed(I_YEL) ) {
      Serial << F("Button: YEL pressed.") << endl;
      while ( buttonAnyPressed() );
      Serial << F("Button: YEL released.") << endl;
    }
  }
}

