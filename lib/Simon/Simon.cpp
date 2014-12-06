#include "Simon.h"

// Need an instance of the Radio Module
RFM12B radio;

byte commsStart(byte nodeID, byte groupID, byte band, byte csPin) {

	// check that the tower payloads aren't the same size.
	int size1=sizeof(towerInstruction);
	int size2=sizeof(towerConfiguration);
	if ( size1==size2 ) {
		Serial << F("Tower instructions are indistinguishable by size!") << endl;
		while (1); // halt
	}

	// error checking:
	if( !(band == RF12_433MHZ || band == RF12_868MHZ || RF12_915MHZ) ) {
		Serial << F("Error! RFM band is out of range!") << endl;
		return(0);
	} else if ( nodeID < 1 || nodeID > 30 ) { // zero and 31 are allowed, but special cases.
		Serial << F("Error! RFM node is out of range!") << endl;
		return(0);
	} else if ( csPin < 8 || csPin > 10 ) {
		Serial << F("Error! RFM cs pin is out of range!") << endl;
		return(0);
	}

	Serial << F("Startup RFM12b radio module.") << endl;
	radio.SetCS(csPin); // assign CS pin before startup
	radio.Initialize(nodeID, band, groupID);

	Serial << F("Startup complete. nodeID: ") << radio.nodeID << endl;

	return(radio.nodeID);
}

byte commsStart() {
	// mirror this code in commsStart and commsSave
	byte offset = 0;
	byte nodeID = EEPROM.read(radioConfigLocation + (offset++));
	byte groupID = EEPROM.read(radioConfigLocation + (offset++));
	byte band = EEPROM.read(radioConfigLocation + (offset++));
	byte csPin = EEPROM.read(radioConfigLocation + (offset++));

	Serial << F("Read EEPROM settings:");
	Serial << F(" NodeID: ") << nodeID;
	Serial << F(" GroupID: ") << groupID;
	Serial << F(" Band: ") << band;
	Serial << F(" csPin: ") << csPin;
	Serial << F(".  Done.") << endl;

	byte retID = commsStart(nodeID, groupID, band, csPin);
	if( retID == 0 ) { // caught an error, almost certainly because we read from uninitialized EEPROM;
		while( ! commsConfigure() ); // configure by keyboard, save results to EEPROM
		return( commsStart() ); // start again from EEPROM
	} else { 
		return( retID );
	}
 }
 
 boolean commsConfigure() {
	Serial << F("Is this node a (T)ower or (C)onsole? (T/C)") << endl;
	while(! Serial.available());
	byte input = Serial.read();
	if( input == 'C' ) {
		commsSave(consoleNodeID);
		return( true );
	}
	
	Serial << F("Which Tower? (1,2,3,4)") << endl;
	while(! Serial.available());
	input = Serial.read();
	switch( input ) {
		case '1': commsSave(towerNodeID[0]); return(true); break;
		case '2': commsSave(towerNodeID[1]); return(true); break;
		case '3': commsSave(towerNodeID[2]); return(true); break;
		case '4': commsSave(towerNodeID[3]); return(true); break;
	}	
	
	// try again, user.
	return( false );
 }

void commsSave(byte nodeID, byte groupID, byte band, byte csPin) {
	Serial << F("Writing EEPROM settings:");
	Serial << F(" NodeID: ") << nodeID;
	Serial << F(" GroupID: ") << groupID;
	Serial << F(" Band: ") << band;
	Serial << F(" csPin: ") << csPin;
	
	// mirror this code in commsStart and commsSave
	byte offset = 0;
	EEPROM.write(radioConfigLocation + (offset++), nodeID);
	EEPROM.write(radioConfigLocation + (offset++), groupID);
	EEPROM.write(radioConfigLocation + (offset++), band);
	EEPROM.write(radioConfigLocation + (offset++), csPin);

	Serial << F(".  Done.") << endl;
}

// ping functions with ACK used to establish network after comms are initialized
boolean commsSendPing(byte nodeID, int waitACK) {
	// use the instruction payload size+1.  It's contents are irrelevant.
	byte payload[sizeof(towerInstruction)+1];

	// send the packet with an ACK request
	radio.Send(nodeID, (const void*)(&payload), sizeof(payload), waitACK>0?true:false);
  
	// maybe wait for ACK
	if( waitACK > 0 ) {
		unsigned long now = millis();
		while( millis() - now <= waitACK ) { // wait for ACK
			if( radio.ACKReceived(nodeID) ) return(true); // got one
		}
		return(false); // didn't get one
	}
	return(true); // with no ACK request, we're done.

}
boolean commsRecvPing() {
	// if nothing is received or it's the wrong size, exit
	if( !(radio.ReceiveComplete() && radio.CRCPass() && radio.GetDataLen() == sizeof(towerInstruction)+1) ) 
		return( false ); 

	if( radio.ACKRequested() ) radio.SendACK(); // send ACK back to sender.

	// return true
	return( true );
}
// calls commsSendPing multiple times and return percent of packets that were ACK'd.
byte commsQuality(byte nodeID, int waitACK) {

	// store the number of ACKs we get back
	byte ACKcount = 0;

	// ping 100 times
	for( int i=0;i<100;i++ ) {
		if( commsSendPing(nodeID, waitACK) ) ACKcount++;
	}

	return( ACKcount );
}


// sets defaults for instruction
void commsDefault(towerInstruction &inst, byte lightLevel, byte fireLevel) {
	for (int i = 0; i < N_COLORS; i++ ) {
		inst.lightLevel[i] = lightLevel;
		inst.fireLevel[i] = fireLevel;
	}
}
// and prints instruction
void commsPrint(towerInstruction &inst) {
	Serial << F("Tower instruction:\t");
	for (byte i; i < N_COLORS; i++) {
		Serial << i << F(": l(") << inst.lightLevel[i] << F(")f(") << inst.fireLevel[i] << F(")\t") ;
	}
	Serial << endl;
}
// receive instruction
boolean commsRecv(towerInstruction &inst) {
	// if nothing is received or it's the wrong size, exit
	if( !(radio.ReceiveComplete() && radio.CRCPass() && radio.GetDataLen() == sizeof(inst)) ) return( false ); 

	// otherwise, save it
	inst = *(towerInstruction*)radio.Data;

	if( radio.ACKRequested() ) radio.SendACK(); // send ACK back to sender, if requested.

	return( true );
}
// send instruction
void commsSend(towerInstruction &inst) {
	// send the packet in broadcast mode
	radio.Send(0, (const void*)(&inst), sizeof(inst), false); 
	// with no ACK, we're done.
}

// set defaults for towerConfiguration
void commsDefault(towerConfiguration &config, byte colorIndex, byte fireIndex, 
					unsigned long minFireTime, unsigned long maxFireTime, 
					unsigned long flameCoolDownTime) {
	config.colorIndex = colorIndex; // all
	config.fireIndex = fireIndex; // all
	config.minFireTime = minFireTime;
	config.maxFireTime = maxFireTime;
	config.flameCoolDownTime = flameCoolDownTime;
};
// and prints current configuration
void commsPrint(towerConfiguration &config) {
	Serial << F("Tower config: ");
	Serial << F("Color(") << config.colorIndex << F(") Fire(") << config.fireIndex << F(") ");
	Serial << F("Flame min(") << config.minFireTime << F(") max(") << config.maxFireTime << F(").") << endl;
}
// receive incoming configuration
boolean commsRecv(towerConfiguration &config) {
	// if nothing is received or it's the wrong size, exit
	if( !(radio.ReceiveComplete() && radio.CRCPass() && radio.GetDataLen() == sizeof(config)) ) return( false ); 

	// otherwise, save it
	config = *(towerConfiguration*)radio.Data;

	// the configurations are very key, so we're going to use an ACK here.
	if( radio.ACKRequested() ) radio.SendACK(); // send ACK back to sender, if requested.

	// return true
	return( true );
}
// send incoming configuration
boolean commsSend(towerConfiguration &config, byte nodeID, int waitACK) {

	// send the packet with an ACK request
	radio.Send(nodeID, (const void*)(&config), sizeof(config), waitACK>0?true:false);
  
	// maybe wait for ACK
	if( waitACK > 0 ) {
		unsigned long now = millis();
		while( millis() - now <= waitACK ) { // wait for ACK
			if( radio.ACKReceived(nodeID) ) return(true); // got one
		}
		return(false); // didn't get one
	}
	return(true); // with no ACK request, we're done.
}

// sets up and transmits Tower configurations
boolean commsConfigTowers(towerConfiguration (&config)[N_TOWERS], byte colorLayout, byte fireLayout,
						unsigned long minFireTime, unsigned long maxFireTime, 
						unsigned long flameCoolDownTime ) {
						
	// pull the layouts together
	byte colorIndex[N_TOWERS];
	byte fireIndex[N_TOWERS];
	for(int i=0;i<N_TOWERS;i++) {
		colorIndex[i] = towerConfigLayouts[colorLayout][i];
		fireIndex[i] = towerConfigLayouts[fireLayout][i];
	}
	
	// store status of transmissions
	boolean configuredOK = true;

	for(int ni=0; ni<N_TOWERS; ni++) {
		// set configurations
		commsDefault(config[ni], colorIndex[ni], fireIndex[ni], minFireTime, maxFireTime, flameCoolDownTime);
		// send configurations	
		Serial << F("Node ") << towerNodeID[ni] << F(" ");
		commsPrint(config[ni]);
		
		int configurePasses = 10; // equivalent to 100*10 ms or 1 sec of configuration send failure
		while( !commsSend(config[ni], towerNodeID[ni], 100) && configurePasses>=0) { // send and wait 0.5 sec for response
			Serial << F("NOTHING from Node ") << towerNodeID[ni] << endl;
			configurePasses --;
		}
		if( configurePasses < 0 ) configuredOK = false; // failed for this node
	}

	return( configuredOK );
}


void commsUnitTestConsole(towerConfiguration (&config)[N_TOWERS], towerInstruction &inst, int numberOfTests) {

	byte passOffSet = 0;

	while ( numberOfTests-- ) { // countdown
		
		byte colorLayout = (passOffSet++) % N_CONFIG;
		byte fireLayout = (passOffSet++) % N_CONFIG;
		Serial << F("Configuring Towers. colorLayout: ") << colorLayout << F(" fireLayout: ") << fireLayout << endl;
		// use this layout
		commsConfigTowers(config, colorLayout, fireLayout, D_MIN_FLAME, D_MAX_FLAME, 10);
		
		// now that configurations are set, send instructions
		for(int nc=0; nc<N_COLORS; nc++) {
		
			// start with the defaults
			commsDefault(inst);
			// adjust the instructions			
			inst.lightLevel[nc] = 255;
			inst.fireLevel[nc] = 255;
			
			Serial << F("Instructing ON for color/fire index: ") << nc << endl;
			commsSend(inst); // send
			delay(1000); // wait for the humans to catch up.
		}
			
	}
}

