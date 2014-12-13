#include "Extern.h"

// sets up external interface
void externStart() {
  Serial << F("Extern startup.") << endl;
  
  // nothing to do, yet.
}

// checks for external traffic; if traffic, responds to it and returns true
boolean externUpdate() {
  // nothing to do, yet.

  return( false );
}

