#include "Network.h"

void Network::begin(nodeID node) {
  Serial << F("Network: begin") << endl;

  this->node = networkStart(node);

  // setup Light comms
  Serial1.begin(115200);
  //start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
  this->ET.begin(details(this->state), &Serial1);
  Serial << F("Network: serial comms with Light module.") << endl;

  // arise, Cthulu
  //radio.Wakeup();  // this was crashing startup

  // check the send time
  Serial << F("Network: system datagram size (bytes)=") << sizeof(this->state) << endl;
  // send.   match Network::update Send syntax exactly
  radio.Send(255, (const void*)(&this->state), sizeof(this->state), false, 0);
  radio.SendWait();
  unsigned long tic = micros();
  radio.Send(255, (const void*)(&this->state), sizeof(this->state), false, 0);
  radio.SendWait();
  unsigned long toc = micros();
  Serial << F("Network: system datagram requires ") << toc - tic << F("us to send.") << endl;

  // save this, bumped slighly and at least 5ms
  this->packetSendInterval = max(5000UL, float(toc - tic) * 1.1);
//  this->packetSendInterval = float(toc - tic) * 1.1;
  Serial << F("Network: sending system datagram every ") << this->packetSendInterval << F("us.") << endl;

  this->resendCount = 10;
  this->sentCount = this->resendCount;
  Serial << F("Network: will resend new packets x") << this->resendCount << endl;

  // Get layout
  int addr = 69;
  color lightLayout[N_COLORS] = {I_RED, I_GRN, I_BLU, I_YEL}, fireLayout[N_COLORS] = {I_RED, I_GRN, I_BLU, I_YEL};
  for ( byte i = 0; i < N_COLORS; i++ ) {
    lightLayout[i] = (color)EEPROM.read(addr+i);
    fireLayout[i] = (color)EEPROM.read(addr+i+N_COLORS);
  }
  
  // sanity check  
  this->layout( lightLayout, fireLayout ); // redundant, but I want the print

  Serial << F("Network: setup complete.") << endl;
}

void Network::layout(color lightLayout[N_COLORS], color fireLayout[N_COLORS]) {
  Serial << F("Network: layout:") << endl;

  // write to EEPROM if there's a change.
  int addr = 69;

  // store it
  for ( byte i = 0; i < N_COLORS; i++ ) {
    this->lightLayout[i] = lightLayout[i];
    Serial << F(" Color ") << i << (" assigned to Tower ") << lightLayout[i] << endl;

    if( EEPROM.read(addr+i) != (byte)lightLayout[i] ) {
      Serial << F("  set EEPROM") << endl;
      EEPROM.write(addr+i, (byte)lightLayout[i]);
    } 
    
    this->fireLayout[i] = fireLayout[i];
    Serial << F(" Fire ") << i << (" assigned to Tower ") << fireLayout[i] << endl;

    if( EEPROM.read(addr+i+N_COLORS) != (byte)fireLayout[i] ) {
      Serial << F("  set EEPROM") << endl;
      EEPROM.write(addr+i+N_COLORS, (byte)fireLayout[i]);
    } 
  }

}

// resends and stuff
void Network::update() {
  // if the sent count exceeds resend count, exit
  if ( this->sentCount >= this->resendCount ) return;

  // track send times
  static unsigned long lastSend = micros();
  unsigned long now = micros();

  // send on an interval
  if( now-lastSend < this->packetSendInterval ) return;

  // if this is the first time we've sent, update the packet number
  if ( this->sentCount == 0 ) {
    this->state.packetNumber++;
    Serial << F("Network::update.  New packet # ") << this->state.packetNumber << endl;
    for( int i=0; i<N_COLORS; i++ )
      Serial << F("  color:" ) << i << F(" red:") << this->state.light[i].red << F(" green:") << this->state.light[i].green << F(" blue:") << this->state.light[i].blue << endl; 
    
  } else {
//    Serial << F("Network::update.  resend # ") << this->sentCount << endl;
  }

  // Radio: send. no ACK, no sleep.
  this->send();
  
  this->sentCount++;

  // record last send time
  lastSend = now;
}

// makes the network do stuff with your stuff
void Network::send(color position, colorInstruction &inst) {
  // change on a delta
  if ( memcmp((void*)(&inst), (void*)(&this->state.light[position]), sizeof(colorInstruction)) != 0 ) {
    this->state.light[position] = inst;
    this->sentCount = 0;
  }
}
void Network::send(color position, fireInstruction &inst) {
  // change on a delta
  if ( memcmp((void*)(&inst), (void*)(&this->state.fire[position]), sizeof(fireInstruction)) != 0 ) {
    this->state.fire[position] = inst;
    this->sentCount = 0;
  }
}
void Network::send(systemMode mode) {
  // change on a delta
  if ( mode != (byte)state.mode ) {
    this->state.mode = (byte)mode;
    this->sentCount = 0;
  }
}
void Network::send(animationInstruction &inst) {
  // change on a delta
  if ( memcmp((void*)(&inst), (void*)(&this->state.animation), sizeof(animationInstruction)) != 0 ) {
    this->state.animation = inst;
    this->sentCount = 0;
  }
}

// internal dispatcher
void Network::send() {

  // Light: send.  handled by dedicated UART hardwre, so will happen in the background.
  ET.sendData();

  // apply physical Tower layout
  systemState towerState;
  // copy out invariants
  towerState.packetNumber = this->state.packetNumber;
  towerState.mode = this->state.mode;
  towerState.animation = this->state.animation;

  for( byte i=0; i<N_COLORS; i++ ) {
    if( this->lightLayout[i] != N_COLORS ) {
      // if single tower are handling single colors
      towerState.light[i] = this->state.light[this->lightLayout[i]];
    } else {
      // towers are handling multiple color instructions
      this->mergeColor(towerState.light[i]);
    }
    if ( this->fireLayout[i] != N_COLORS ) {
      towerState.fire[i] = this->state.fire[this->fireLayout[i]];
    } else {
      // towers are handling multiple fire instructions
      this->mergeFire(towerState.fire[i]);
    }
  }

  radio.Send((byte)BROADCAST, (const void*)(&towerState), sizeof(towerState), false, 0);
}

// we sum up the lighting instructions
void Network::mergeColor(colorInstruction &inst) {
  unsigned long red = 0, green = 0, blue = 0;
  for ( byte i = 0; i < N_COLORS; i++ ) {
    // summation
    red += this->state.light[i].red;
    green += this->state.light[i].green;
    blue += this->state.light[i].blue;
    
    // maybe maxima would be more appropriate?
//    red = max( red, this->state.light[i].red );
//    green = max( green, this->state.light[i].green );
//    blue = max( blue, this->state.light[i].blue );
  }

  inst.red = constrain(red, 0, 255);
  inst.green = constrain(green, 0, 255);
  inst.blue = constrain(blue, 0, 255);
}

// we sum up the fire instructions
void Network::mergeFire(fireInstruction &inst) {
  unsigned long duration = 0, effect = 0;
  for ( byte i = 0; i < N_COLORS; i++ ) {
    // summation
    duration += this->state.fire[i].duration;
    effect += this->state.fire[i].effect;
    
    // maybe maxima would be more appropriate?
//    duration = max( duration, this->state.fire[i].duration );
//    effect = max( effect, this->state.fire[i].effect );  
  }

  inst.duration = constrain(duration, 0, 255);
  inst.effect = constrain(effect, veryRich, veryLean);
}

// starts the radio
nodeID Network::networkStart(nodeID node) {

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

    //    radio.Initialize(node, band, groupID, 0, 0x7F); // 38300 bps
    radio.Initialize(node, band, groupID, 0, 0x02); // 115200 bps

    Serial << F("Network: RFM12b radio module startup complete. ") << endl;

    return ( (nodeID)node );
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

