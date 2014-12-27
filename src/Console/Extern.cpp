#include "Extern.h"

// this is where the lights and fire instructions to Towers are placed
// defined in Simon_Comms.h and instantiated in Tower.cpp
extern towerInstruction inst;
// from Simon_Comms.h:
// typedef struct {
//	byte lightLevel[N_COLORS]; // 0..255.  maps to analogWrite->light level
//	byte fireLevel[N_COLORS]; // 0..255.  maps to timer->fire duration
// } towerInstruction;
// structure stores how Towers acts on those towerInstructions

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

    // check cases
    if ( radio.GetDataLen() == sizeof(inst) ) {

      // save instruction for lights/flame
      inst = *(towerInstruction*)radio.GetData();
      // do it.
      externDoInstruction();

    }

    // check for ACK request
    if ( radio.ACKRequested() )
      radio.SendACK();

  }

  return ( !externTimeout.check() );
}

// takes a towerInstruction and plays that out on the Console.  meh.
void externDoInstruction() {
  // very straight-forward.  Emulates button presses on the Console according to towerInstruction light settings.
  pixelSet(I_RED, inst.lightLevel[I_RED]);
  pixelSet(I_GRN, inst.lightLevel[I_GRN]);
  pixelSet(I_BLU, inst.lightLevel[I_BLU]);
  pixelSet(I_YEL, inst.lightLevel[I_YEL]);

  // towerInstruction fireLevel is ignored on the Console.
}
