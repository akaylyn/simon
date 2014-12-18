#ifndef Simon_Comms_h
#define Simon_Comms_h

#include "Simon_Indexes.h"

// default minimum and maximum solenoid opening time
#define D_MIN_FLAME 50UL // ms
#define D_MAX_FLAME 50UL // ms
// default interval between the flame solenoid closing and reopening again
#define D_FLAME_COOLDOWN 1000UL // ms

#include <Arduino.h>
#include <Streaming.h> // <<-style printing

#define D_GROUP_ID 188// default RFM group
#define D_CS_PIN 10 // default SS pin for RFM module
#define D_WAIT_ACK 50 // default wait time for ACK receipt, ms

#include <RFM12B.h> // radio board

#include <EEPROM.h> // for saving and loading radio settings
// EEPROM location for radio settings.
const byte radioConfigLocation = 42;
// EEPROM location for towerConfiguration settings.
const byte towerConfigLocation = 69;

// RFM12b comms
// Group ID = 188
// Simon 1:10
// Giles 11-20
// Clouds 21-210

// defined nodes in the network.  should be used to test results of commsStart().
const byte consoleNodeID = 1;
// how many towers
#define N_TOWERS 4
const byte towerNodeID[N_TOWERS] = {2,3,4,5};

// STARTUP

// start rfm12b communications

// uses EEPROM values for configuration; returns NODEID after configuration or zero if failed.
byte commsStart(); 
// sets configuration by argument; returns NODEID after configuration or zero if failed.
byte commsStart(byte setNodeID, byte groupID=D_GROUP_ID, byte band=RF12_915MHZ, byte csPin=D_CS_PIN); 
// saves configuration to EEPROM for later commsStart() use.
void commsSave(byte nodeID, byte groupID=D_GROUP_ID, byte band=RF12_915MHZ, byte csPin=D_CS_PIN);
// ping functions with ACK used to establish network after comms are initialized
boolean commsSendPing(byte nodeID, int waitACK=D_WAIT_ACK);

// structure definition for information passed between Console and Towers

// CONFIGURATION

// during startup, Console defines which Tower(s) respond to color and fire signals in towerInstruction:
typedef struct {
	byte colorIndex; // what color(s) should a tower respond to?
	byte fireIndex;  // what color(s) should a tower respond to?
	// 0..3 for specific color. 4=all. 5=none.
	unsigned long minFireTime; // maps fireLevel to ms
	unsigned long maxFireTime; // maps fireLevel to ms
	unsigned long flameCoolDownTime; // enforce a flame shutdown of this interval between poofs
} towerConfiguration;
// prints configuration
void commsPrint(towerConfiguration &config, byte NodeID);
// sets defaults for instruction
void commsDefault(towerConfiguration &config, byte colorIndex=I_ALL, byte fireIndex=I_NONE, // no fire by default, SAFETY.
					unsigned long minFireTime=D_MIN_FLAME, unsigned long maxFireTime=D_MAX_FLAME, 
					unsigned long flameCoolDownTime=D_FLAME_COOLDOWN);
// send configuration from Console to Tower nodeID.
boolean commsSend(towerConfiguration &config, byte nodeID, int waitACK=D_WAIT_ACK);
// saves configuration to EEPROM.  Towers use this to restart to last configuration state.
void commsSave(towerConfiguration &config);
// reads configuration from EEPROM.  Towers use this to save last configuration state.
void commsLoad(towerConfiguration &config);

// GAMEPLAY 

// during gameplay, this is the information passed from Console to Towers:
typedef struct {
	byte lightLevel[N_COLORS]; // 0..255.  maps to analogWrite->light level
	byte fireLevel[N_COLORS]; // 0..255.  maps to timer->fire duration
} towerInstruction;
// set defaults for configuration
void commsDefault(towerInstruction &inst, byte lightLevel=0, byte fireLevel=0);
// prints instruction to Serial
void commsPrint(towerInstruction &inst);
// send instruction from Console.  Broadcast mode, so every tower receives.
void commsSend(towerInstruction &inst);

	
#endif
