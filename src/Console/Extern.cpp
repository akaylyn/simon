#include "Extern.h"

// Need an instance of the Radio Module.  Instantiated in Tower.cpp.
extern RFM12B radio;

// sets up external interface
void externStart() {
  Serial << F("Extern: startup.") << endl;

  // nothing to do, yet.
}

// checks for external traffic; if traffic, responds to it and returns true
boolean externUpdate() {
  // once we get radio comms, wait this long  before returning false from externUpdate.
  static Metro externTimeout(EXTERNAL_COMMS_TIMEOUT);

  // important that we check the buffer before sending, or we'll overwrite stuff
  if ( radio.ReceiveComplete() && radio.CRCPass() ) {
    // we have radio comms
    externTimeout.reset();

    // message storage
    static towerInstruction tInst;
    
    // check cases
    if ( radio.GetDataLen() == sizeof(tInst) ) {

      // save instruction for lights/flame
      tInst = *(towerInstruction*)radio.GetData();
      // do it.
      light.sendInstruction(tInst);
        
    }

    // check for ACK request
    if ( radio.ACKRequested() )
      radio.SendACK();

  }

  return ( !externTimeout.check() );
}


