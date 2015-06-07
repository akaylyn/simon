#include "Instruction.h"

void Instruction::begin(nodeID node) {
  Serial << F("Instruction::begin") << endl;

  this->node = networkStart(node);
}

void Instruction::update(colorInstruction &colorInst, fireInstruction &fireInst, modeSwitchInstruction &modeInst) {
  // check for comms traffic
  if ( radio.receiveDone() ) {
    // process it.
    if ( radio.DATALEN == sizeof(colorInst) ) {
      // check to see if the instructions have changed?
      //if ( memcmp((void*)(&colorInst), (void*)radio.DATA, sizeof(colorInstruction)) != 0 ) {
        // save instruction for light
        colorInst = *(colorInstruction*)radio.DATA;

    Serial << F("Rx instruction. R:") << colorInst.red << F(" G:") << colorInst.green << F(" B:") << colorInst.blue << endl;
 //       Serial << endl << F("C") << endl;
   //   } else {
   //     Serial << F("c");
   //   }
    } else if ( radio.DATALEN == sizeof(fireInst) ) {
      // check to see if the instructions have changed?
   //   if ( memcmp((void*)(&fireInst), (void*)radio.DATA, sizeof(fireInstruction)) != 0 ) {
        // save instruction for fire
        fireInst = *(fireInstruction*)radio.DATA;

  //      Serial << endl << F("F") << endl;
        Serial << F("Rx: fire instruction. D:") << fireInst.flame << F(" E:") << fireInst.effect  << endl;
   //   } else {
   //     Serial << F("f");
   //   }
    } else if ( radio.DATALEN == sizeof(commsCheckInstruction) ) {
      // ping received.
      commsCheckInstruction ping;
      // save instruction 
      ping = *(commsCheckInstruction*)radio.DATA;
      
      Serial << F("Ping: ") << ping.packetNumber << F(" of ") << ping.packetTotal << endl;
    } else if ( radio.DATALEN == sizeof(modeSwitchInstruction) ) {
      modeInst = *(modeSwitchInstruction*)radio.DATA;
    }
  }
  
}

// starts the radio
nodeID Instruction::networkStart(nodeID node) {
  // EEPROM location for radio settings.
  const byte radioConfigLocation = 42;

  if ( node == BROADCAST ) {
    // try to recover settings from EEPROM
    byte offset = 0;
    byte node = EEPROM.read(radioConfigLocation + (offset++));
    byte groupID = EEPROM.read(radioConfigLocation + (offset++));
    byte band = EEPROM.read(radioConfigLocation + (offset++));

    Serial << F("Tower: Startup RFM69HW radio module. ");
    Serial << F(" NodeID: ") << node;
    Serial << F(" GroupID: ") << groupID;
    Serial << F(" Band: ") << band;
    Serial << endl;

    radio.initialize(band, node, groupID);
    radio.setHighPower(); // for HW boards.
    radio.promiscuous(true); // so broadcasts are received.

    Serial << F("Tower: RFM69HW radio module startup complete. ");

    return( (nodeID)node );
  } else {
    Serial << F("Tower: writing EEPROM (bootstrapping).") << endl;
    // then EEPROM isn't configured correctly.
    byte offset = 0;
    EEPROM.write(radioConfigLocation + (offset++), node);
    EEPROM.write(radioConfigLocation + (offset++), D_GROUP_ID);
    EEPROM.write(radioConfigLocation + (offset++), RF69_915MHZ);

    return ( networkStart(BROADCAST) ); // go again after EEPROM save
  }
}

byte Instruction::getNodeID() {
  return( this->node );
}

