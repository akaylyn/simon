#include "Network.h"

void Network::begin(nodeID node, byte sendCount) {
  Serial << F("Network: begin") << endl;

  this->sendCount = sendCount;
  this->node = networkStart(node);

  // arise, Cthulu
  radio.Wakeup();
  
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
    
    // save this, bumped slighly
    this->packetSendTime[i] = float(toc-tic)*1.1;
  }
  
  // and take a moment to check heap+stack remaining
  extern int freeRam();
  Serial << F("Network: free RAM: ") << freeRam() << endl;
  
  Serial << F("20 packet ping to BROADCAST") << endl;
  this->ping(20);
/*
  Serial << F("20 packet ping to Tower1") << endl;
  this->ping(20, TOWER1);
  Serial << F("20 packet ping to Tower2") << endl;
  this->ping(20, TOWER2);
  Serial << F("20 packet ping to Tower3") << endl;
  this->ping(20, TOWER3);
  Serial << F("20 packet ping to Tower4") << endl;
  this->ping(20, TOWER4);
  */
  
  Serial << F("Network: free RAM: ") << freeRam() << endl;
  
  Serial << F("Network: setup complete.") << endl;
  delay(1000);

}

// resends and stuff
void Network::update() {
  // track send times
  static Metro radioReady(3UL);
  
  // is there something queued?
  if( !que.isEmpty()  ) {    
    // and the resend interval has elapsed
    if( !radioReady.check() ) return;
    radioReady.reset();
    
//    Serial << F("Network: poped que entry") << endl;
//    // and take a moment to check heap+stack remaining
//    extern int freeRam();
//    Serial << F("Network: free RAM: ") << freeRam() << endl;
    
    // yep. pop a que entry.
    sendBuffer send = que.pop();
    
    // send. no ACK, no sleep.
    radio.Send(send.address, send.buffer, send.size, false, 0);
    
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
  send( (const void*)(&inst), sizeof(inst), node, dropConflictingInstructions );
}
void Network::send(fireInstruction &inst, nodeID node, boolean dropConflictingInstructions) { 
  send( (const void*)(&inst), sizeof(inst), node, dropConflictingInstructions );
}
void Network::send(modeSwitchInstruction &inst, nodeID node, boolean dropConflictingInstructions) {
  send( (const void*)(&inst), sizeof(inst), node, dropConflictingInstructions );
}
void Network::send(commsCheckInstruction &inst, nodeID node, boolean dropConflictingInstructions) {
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
  this->update();
}

void Network::ping(int count, nodeID node) {
  commsCheckInstruction packet[count];
  
  // don't resend
  byte saveSendCount = this->sendCount;
  this->sendCount = 0;
  
  for( int i=0; i<count; i++ ) {
    packet[i].packetTotal = count;
    packet[i].packetNumber = i;
    
//    radio.Send((byte)node, (const void*)(&packet), sizeof(packet)); // don't drop
//    delay(2); // need 2ms between packets

    // use the same sends, just with que and no replacement
    this->send(packet[i], node, false);
  }
  
  // wait until sent
  while( !que.isEmpty() ) this->update();
  
  // reset sendCount  
  this->sendCount = saveSendCount;

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

