#ifndef Simon_h
#define Simon_h

// how many colors
#define N_COLORS 4
// specific color indices to arrays
#define I_RED 0
#define I_GRN 1
#define I_BLU 2
#define I_YEL 3
#define I_ALL N_COLORS // special case: react to requests on all channels
#define I_NONE N_COLORS+1 // special case: react to NO requests of any kind.

// how many towers
#define N_TOWERS 4

// default minimum and maximum solenoid opening time
#define D_MIN_FLAME 50UL // ms
#define D_MAX_FLAME 50UL // ms
// default interval between the flame solenoid closing and reopening again
#define D_FLAME_COOLDOWN 1000UL // ms

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <RFM12B.h> // radio board
#include <EEPROM.h> // for saving and loading radio settings

// EEPROM location for radio settings.
const byte radioConfigLocation = 42;

// defined nodes in the network.  should be used to test results of commsStart().
const byte consoleNodeID = 1;
const byte towerNodeID[N_TOWERS] = {11, 12, 13, 14};

#define D_GROUP_ID 69 // default RFM group

#define D_CS_PIN 10 // default SS pin for RFM module

#define D_WAIT_ACK 30 // default wait time for ACK receipt

// define tower configurations
#define N_CONFIG 3
#define TOWER_ALL 0
#define TOWER_ONE 1
#define TOWER_NONE 2
const byte towerConfigLayouts[N_CONFIG][N_TOWERS] = {
	{I_ALL, I_ALL, I_ALL, I_ALL}, // all towers will respond
	{I_RED, I_GRN, I_BLU, I_YEL}, // specific towers will respond
	{I_NONE, I_NONE, I_NONE, I_NONE} // no  towers will respond
};

// start rfm12b communications

// uses EEPROM values for configuration; returns NODEID after configuration or zero if failed.
byte commsStart(); 
// sets configuration by argument; returns NODEID after configuration or zero if failed.
byte commsStart(byte setNodeID, byte groupID=D_GROUP_ID, byte band=RF12_915MHZ, byte csPin=D_CS_PIN); 
// sets up the configuration through Serial
boolean commsConfigure();
// saves configuration to EEPROM for later commsStart() use.
void commsSave(byte nodeID, byte groupID=D_GROUP_ID, byte band=RF12_915MHZ, byte csPin=D_CS_PIN);

// ping functions with ACK used to establish network after comms are initialized
boolean commsSendPing(byte nodeID, int waitACK=D_WAIT_ACK);
boolean commsRecvPing();
// calls commsSendPing multiple times and return percent of packets that were ACK'd.
byte commsQuality(byte nodeID, int waitACK=D_WAIT_ACK);

// structure definition for information passed between Console and Towers

// during gameplay, this is the information passed from Console to Towers:
typedef struct {
	byte lightLevel[N_COLORS]; // 0..255.  maps to analogWrite->light level
	byte fireLevel[N_COLORS]; // 0..255.  maps to timer->fire duration
} towerInstruction;
// set defaults for configuration
void commsDefault(towerInstruction &inst, byte lightLevel=0, byte fireLevel=0);
// prints instruction
void commsPrint(towerInstruction &inst);
// receive instruction
boolean commsRecv(towerInstruction &inst);
// send instruction
void commsSend(towerInstruction &inst);

// during startup, Console defines which Tower(s) respond to color and fire signals in towerInstruction:
// define start up payload:
typedef struct {
	byte colorIndex; // what color(s) should a tower respond to?
	byte fireIndex;  // what color(s) should a tower respond to?
	// 0..3 for specific color. 4=all. 5=none.
	unsigned long minFireTime; // maps fireLevel to ms
	unsigned long maxFireTime; // maps fireLevel to ms
	unsigned long flameCoolDownTime; // enforce a flame shutdown of this interval between poofs
} towerConfiguration;
// prints configuration
void commsPrint(towerConfiguration &config);
// sets defaults for instruction
void commsDefault(towerConfiguration &config, byte colorIndex=I_ALL, byte fireIndex=I_ALL, 
					unsigned long minFireTime=D_MIN_FLAME, unsigned long maxFireTime=D_MAX_FLAME, 
					unsigned long flameCoolDownTime=D_FLAME_COOLDOWN);
// receive configuration
boolean commsRecv(towerConfiguration &config);
// send configuration
boolean commsSend(towerConfiguration &config, byte nodeID, int waitACK=D_WAIT_ACK);
// sets up and transmits Tower configuration from the Console side
boolean commsConfigTowers(
	towerConfiguration (&config)[N_TOWERS], 
	byte colorLayout=TOWER_ALL, 
	byte fireLayout=TOWER_ALL, 
	unsigned long minFireTime=D_MIN_FLAME, unsigned long maxFireTime=D_MAX_FLAME, 
	unsigned long flameCoolDownTime=D_FLAME_COOLDOWN );

// performs a unit test, from the Console side
void commsUnitTestConsole(towerConfiguration (&config)[N_TOWERS], towerInstruction &inst, int numberOfTests=10);

	
#endif
