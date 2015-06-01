#include "Instruction.h"

void Instruction::Instruction(byte nodeID) {
  this->nodeID = networkStart(nodeID);
}

boolean Instruction::update(towerInstruction &inst) {
  // track if we have an update for outputs
  boolean haveUpdate = false;

  // check for comms traffic
  if ( radio.receiveDone() ) {
    // process it.
    if ( radio.DATALEN == sizeof(inst) ) {
      // check to see if the instructions have changed?
      if ( memcmp((void*)(&inst), (void*)radio.DATA, sizeof(inst)) != 0 ) {
        // save instruction for lights/flame
        inst = *(towerInstruction*)radio.DATA;
        return(true);

        Serial << F("I") << endl;
      } else {
        Serial << F("i") << endl;
      }
    } else if ( radio.DATALEN == sizeof(inst) + 1 ) {
      // ping received.
      Serial << F("ping received.") << endl;
    }
  }
  
  return(false);
}

// starts the radio
byte Instruction::networkStart(byte nodeID) {
  // EEPROM location for radio settings.
  const byte radioConfigLocation = 42;

  if ( nodeID == 0 ) {
    // try to recover settings from EEPROM
    byte offset = 0;
    byte nodeID = EEPROM.read(radioConfigLocation + (offset++));
    byte groupID = EEPROM.read(radioConfigLocation + (offset++));
    byte band = EEPROM.read(radioConfigLocation + (offset++));

    Serial << F("Tower: Startup RFM69HW radio module. ");
    Serial << F(" NodeID: ") << nodeID;
    Serial << F(" GroupID: ") << groupID;
    Serial << F(" Band: ") << band;
    Serial << endl;

    radio.initialize(band, nodeID, groupID);
    radio.setHighPower(); // for HW boards.
    radio.promiscuous(true); // so broadcasts are received.

    Serial << F("Tower: RFM69HW radio module startup complete. ");

    return (nodeID);
  } else {
    Serial << F("Tower: writing EEPROM (bootstrapping).") << endl;
    // then EEPROM isn't configured correctly.
    byte offset = 0;
    EEPROM.write(radioConfigLocation + (offset++), nodeID);
    EEPROM.write(radioConfigLocation + (offset++), D_GROUP_ID);
    EEPROM.write(radioConfigLocation + (offset++), RF69_915MHZ);

    return ( networkStart(0) ); // go again after EEPROM save
  }
}

