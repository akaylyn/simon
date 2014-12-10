#include "Simon_Comms.h"

// instantiated from main codes
extern RFM12B radio;

// STARTUP

byte commsStart(byte nodeID, byte groupID, byte band, byte csPin) {

	// check that the tower payloads aren't the same size.
	int size1=sizeof(towerInstruction);
	int size2=sizeof(towerConfiguration);
	if ( size1==size2 ) {
		Serial << F("Tower instructions are indistinguishable by size!") << endl;
		while (1); // halt
	}

	// error checking:
	if( !(band == RF12_433MHZ || band == RF12_868MHZ || band == RF12_915MHZ) ) {
		Serial << F("Error! RFM band is out of range!") << endl;
		return(0);
	} else if ( nodeID < 1 || nodeID > 30 ) { // zero and 31 are allowed, but special cases.
		Serial << F("Error! RFM node is out of range!") << endl;
		return(0);
	} 
	
	Serial << F("Startup RFM12b radio module. ");
	Serial << F(" NodeID: ") << nodeID;
	Serial << F(" GroupID: ") << groupID;
	Serial << F(" Band: ") << band;
	Serial << F(" csPin: ") << csPin;
	Serial << endl;

	radio.Initialize(nodeID, band, groupID);

	return(radio.nodeID);
}
byte commsStart() {
	Serial << F("Reading radio settings from EEPROM.") << endl;
	// mirror this code in commsStart and commsSave
	byte offset = 0;
	byte nodeID = EEPROM.read(radioConfigLocation + (offset++));
	byte groupID = EEPROM.read(radioConfigLocation + (offset++));
	byte band = EEPROM.read(radioConfigLocation + (offset++));
	byte csPin = EEPROM.read(radioConfigLocation + (offset++));

	return( commsStart(nodeID, groupID, band, csPin) );
}
void commsSave(byte nodeID, byte groupID, byte band, byte csPin) {
	Serial << F("Writing radio settings to EEPROM.") << endl;
	// mirror this code in commsStart and commsSave
	byte offset = 0;
	EEPROM.write(radioConfigLocation + (offset++), nodeID);
	EEPROM.write(radioConfigLocation + (offset++), groupID);
	EEPROM.write(radioConfigLocation + (offset++), band);
	EEPROM.write(radioConfigLocation + (offset++), csPin);
}
// ping functions with ACK used to establish network after comms are initialized
boolean commsSendPing(byte nodeID, int waitACK) {
	// use the instruction payload size+1.  It's contents are irrelevant.
	byte payload[sizeof(towerInstruction)+1];

	// send the packet with an ACK request
	radio.Send(nodeID, (const void*)(&payload), sizeof(payload), waitACK>0?true:false);
	radio.SendWait(); // wait for it to get out the door.
	
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

// CONFIGURATION

// set defaults for towerConfiguration
void commsDefault(towerConfiguration &config, byte colorIndex, byte fireIndex, 
					unsigned long minFireTime, unsigned long maxFireTime, 
					unsigned long flameCoolDownTime) {
	config.colorIndex = colorIndex; 
	config.fireIndex = fireIndex; 
	config.minFireTime = minFireTime;
	config.maxFireTime = maxFireTime;
	config.flameCoolDownTime = flameCoolDownTime;
};
// and prints current configuration
void commsPrint(towerConfiguration &config, byte nodeID) {
	Serial << F("Tower (") << nodeID << F(") config: ");
	Serial << F("Color(") << config.colorIndex << F(") Fire(") << config.fireIndex << F(") ");
	Serial << F("Flame min(") << config.minFireTime << F(") max(") << config.maxFireTime << F(").") << endl;
}
// send configuration
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
// saves configuration to EEPROM
void commsSave(towerConfiguration &config) { 
	Serial << F("Writing towerConfiguration to EEPROM.") << endl;
	eeprom_write_block((const void*)&config, (void*)towerConfigLocation, sizeof(config));
}
// reads configuration from EEPROM
void commsLoad(towerConfiguration &config) {
	Serial << F("Reading towerConfiguration from EEPROM.") << endl;
	eeprom_read_block((void*)&config, (void*)towerConfigLocation, sizeof(config));
}

// GAMEPLAY

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
// send instruction
void commsSend(towerInstruction &inst) {
	// send the packet in broadcast mode
	radio.Send(0, (const void*)(&inst), sizeof(inst), false); 
	// with no ACK, we're done.
}



