#include "Network.h"

void Network::begin(nodeID node, unsigned long sendInterval, byte sendCount) {
  Serial << F("Network: begin") << endl;

  this->sendInterval = sendInterval;
  this->sendCount = sendCount;
  this->node = networkStart(node);
}

// resends and stuff
void Network::update() {
  // send on an interval
  static Metro readyToSend(this->sendInterval);

  // if there's something queued and the radio is available
  if( !que.isEmpty() ) { // && radio.CanSend()
    // check to see if the resend interval has elapsed
    Serial << F("Network: que has entries") << endl;
    if( readyToSend.check() ) {
      // pop a que entry
      sendBuffer send = que.pop();
      // send.
      radio.Send(send.address, send.buffer, send.size);
      // increment sendCount
      send.sendCount++;
      Serial << F("Network: popped.  sendCount=") << send.sendCount << endl;
      // we might need to reque
      if( send.sendCount <= this->sendCount ) { 
        que.push(send);
        Serial << F("Network: reque") << endl;
      }
    }
  }
}

// makes the network do stuff with your stuff
void Network::send(colorInstruction &inst, nodeID node) {
  Serial << F("send: r:") << inst.red << F(" g:") << inst.green << F(" b:") << inst.blue << endl;

  send( (const void*)(&inst), sizeof(inst), node );
}
void Network::send(fireInstruction &inst, nodeID node) {
  send( (const void*)(&inst), sizeof(inst), node );
}
void Network::send(modeSwitchInstruction &inst, nodeID node) {
  send( (const void*)(&inst), sizeof(inst), node );
}
void Network::send(commsCheckInstruction &inst, nodeID node) {
  send( (const void*)(&inst), sizeof(inst), node );
}
// and are really just overloaded helpers for this
void Network::send(const void* buffer, byte bufferSize, nodeID node) {
  // whack it
  sendBuffer buff;
  buff.address = (byte)node;
  buff.buffer = buffer;
  buff.size = bufferSize;
  buff.sendCount = 0;
  // stack it
  que.push(buff);
  Serial << F("Network: enqued") << endl;
  // let it go
  update();
}

void Network::ping(int count, nodeID node) {
  commsCheckInstruction packet;
  packet.packetTotal = count;
  
  for( int i=0; i<count; i++ ) {
    packet.packetNumber = i;
    send(packet, node); // to everyone
  }
}

// starts the radio
nodeID Network::networkStart(nodeID node) {
  
  // check that the tower payloads aren't the same size.
  for ( int i = 0; i < N_DATAGRAMS; i++ ) {
    for ( int j = i + 1; j < N_DATAGRAMS; j++ ) {
      if ( instructionSizes[i] == instructionSizes[j] ) {
        Serial << F("Network: radio instructions are indistinguishable by size!  Halting.") << endl;
        Serial << F("Datagrams matched by size: ") << i << F(" and ") << j << endl;
        while (1); // halt
      }
    }
  }

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

    radio.Initialize(node, band, groupID);

    Serial << F("Network: RFM12b radio module startup complete. ");

    return( (nodeID)node );
  } else {
    Serial << F("Network: writing EEPROM (bootstrapping).") << endl;
    // then EEPROM isn't configured correctly.
    byte offset = 0;
    EEPROM.write(radioConfigLocation + (offset++), node);
    EEPROM.write(radioConfigLocation + (offset++), D_GROUP_ID);
    EEPROM.write(radioConfigLocation + (offset++), RF12_915MHZ);

    return ( networkStart(BROADCAST) ); // go again after EEPROM save
  }
}

Network network;

