#include "Network.h"

void Network::begin(nodeID node, byte sendCount) {
  Serial << F("Network: begin") << endl;

  this->sendCount = sendCount;
  this->node = networkStart(node);

  // check the send times
  for ( int i = 0; i < N_DATAGRAMS; i++ ) {
    Serial << F(" Datagram ") << i << F(" size (bytes)=") << instructionSizes[i];
  
    // make a buffer
    byte sendBuffer[i];
    unsigned long tic = micros();
    // send.   match Network::update Send syntax exactly
    radio.Send(255, sendBuffer, sizeof(sendBuffer));
    radio.SendWait();
    unsigned long toc = micros();
        
    Serial << F(" requires ") << toc-tic << F("us to send.") << endl;
  }
}

// resends and stuff
void Network::update() {
  // is there something queued?
  if( !que.isEmpty() ) { 
    // and the resend interval has elapsed
//    Serial << F("Network: que has entries") << endl;
    
    // yep. pop a que entry.
    sendBuffer send = que.pop();
    
    // send. no ACK, no wait.
    radio.Send(send.address, send.buffer, send.size);

    // increment sendCount
    send.sendCount++;
//    Serial << F("Network: popped.  sendCount=") << send.sendCount << endl;
    // we might need to reque
    if( send.sendCount <= this->sendCount ) { 
      que.push(send);
//      Serial << F("Network: reque") << endl;
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
    } else {
      // otherwise, drop it.
//      Serial << F("dropped conflicting que entry") << endl;
    }
  }
}

// makes the network do stuff with your stuff
void Network::send(colorInstruction &inst, nodeID node, boolean dropConflictingInstructions) {
//  Serial << F("send: r:") << inst.red << F(" g:") << inst.green << F(" b:") << inst.blue << endl;
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

//  Serial << F("Network: enqued") << endl;
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

void Network::clear() {
  while( !que.isEmpty() ) que.pop();
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

    radio.Initialize(node, band, groupID, 0, 0x02); // 115200 bps

    Serial << F("Network: RFM12b radio module startup complete. ") << endl;

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

