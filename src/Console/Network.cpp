#include "Network.h"

void Network::begin(nodeID node, unsigned long sendInterval, byte sendCount) {
  Serial << F("Network: begin") << endl;

  this->sendCount = sendCount;
  this->node = networkStart(node);
  
  // try to size sendInterval to cover the largest datagram send time.
  byte largestDG = 0;
  for ( int i = 0; i < N_DATAGRAMS; i++ ) largestDG = max(largestDG, instructionSizes[i]);
  Serial << F("Network: largest datagrame size=") << largestDG << endl;
  
  // make a buffer
  byte sendBuffer[largestDG];
  unsigned long tic = micros();
  // send.  no ACK, no wait.  match Network::update Send syntax exactly
  radio.Send(255, sendBuffer, sizeof(sendBuffer), false, 0);
  // now wait.
  radio.SendWait();
  unsigned long toc = micros();
        
  Serial << F("Network: largest datagram requires ") << toc-tic << F("us to send.") << endl;
  unsigned long minSendInterval = 1.05*float(toc-tic); // bumped up slightly
  
  this->sendInterval = max(minSendInterval, 1000UL*sendInterval);
  Serial << F("Network: will send datagrams every ") << this->sendInterval << F("us.") << endl;
  
}

// resends and stuff
void Network::update() {
  // send on an interval.  microsecond resolution, so rolling-our-own Metro
  static unsigned long lastSendTime = micros();

  // is there something queued?
  if( !que.isEmpty() ) { // && radio.CanSend()
    // and the resend interval has elapsed
    Serial << F("Network: que has entries") << endl;
    
    // has sendInterval time elapsed since last send?
    unsigned long now = micros();
    if( now - lastSendTime >= this->sendInterval ) {
      // yep. pop a que entry.
      sendBuffer send = que.pop();
      
      // send. no ACK, no wait.
      radio.Send(send.address, send.buffer, send.size, false, 0);

      // increment sendCount
      send.sendCount++;
      Serial << F("Network: popped.  sendCount=") << send.sendCount << endl;
      // we might need to reque
      if( send.sendCount <= this->sendCount ) { 
        que.push(send);
        Serial << F("Network: reque") << endl;
      }
      
      // record that we just sent
      lastSendTime = now;
    }
  }
}

// de-queue conflicting network sends
void Network::dropQueEntries(int size, nodeID node) {
  // how many entries are there?
  int entries = que.count();
  // cycle through the que, removing any that match size and node
  for( int i=0; i<entries; i++ ) {
    sendBuffer entry = que.pop();
    if( entry.address != (byte)node || entry.size != size ) {
      que.push(entry);
    }
    // otherwise, drop it.
  }
}

// makes the network do stuff with your stuff
void Network::send(colorInstruction &inst, nodeID node, boolean dropConflictingInstructions) {
  Serial << F("send: r:") << inst.red << F(" g:") << inst.green << F(" b:") << inst.blue << endl;
  if( dropConflictingInstructions ) dropQueEntries(sizeof(inst), node);
  send( (const void*)(&inst), sizeof(inst), node, dropConflictingInstructions );
}
void Network::send(fireInstruction &inst, nodeID node, boolean dropConflictingInstructions) {
  
  send( (const void*)(&inst), sizeof(inst), node, dropConflictingInstructions );
}
void Network::send(modeSwitchInstruction &inst, nodeID node, boolean dropConflictingInstructions) {
  if( dropConflictingInstructions ) dropQueEntries(sizeof(inst), node);
  send( (const void*)(&inst), sizeof(inst), node, dropConflictingInstructions );
}
void Network::send(commsCheckInstruction &inst, nodeID node, boolean dropConflictingInstructions) {
  if( dropConflictingInstructions ) dropQueEntries(sizeof(inst), node);
  send( (const void*)(&inst), sizeof(inst), node, dropConflictingInstructions );
}
// and are really just overloaded helpers for this
void Network::send(const void* buffer, byte bufferSize, nodeID node, boolean dropConflictingInstructions) {
  
  // if requested, drop any que entries of the same message type to the same node
  if( dropConflictingInstructions ) dropQueEntries(bufferSize, node);
  
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

