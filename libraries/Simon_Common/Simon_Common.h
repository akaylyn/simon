#ifndef Simon_Common_h
#define Simon_Common_h

// how many colors
#define N_COLORS 4
// specific color indices to arrays
#define I_RED 0
#define I_GRN 1
#define I_BLU 2
#define I_YEL 3
#define I_ALL N_COLORS // special case: react to requests on all channels
#define I_NONE N_COLORS+1 // special case: react to NO requests of any kind.

// during startup, Console defines which Tower(s) respond to color and fire signals in towerInstruction:
typedef struct {
	byte colorIndex; // what color(s) should a tower respond to?
	byte fireIndex;  // what color(s) should a tower respond to?
	// I_RED, I_GRN, I_BLU, I_YEL for specific color. 
	// I_ALL for all.
	// I_NONE for none.
	unsigned long minFireTime; // maps fireLevel to ms
	unsigned long maxFireTime; // maps fireLevel to ms
	unsigned long flameCoolDownTime; // enforce a flame shutdown of this interval between poofs
} towerConfiguration;
// EEPROM location for towerConfiguration settings.
const byte towerConfigLocation = 69;

// during gameplay, this is the information passed from Console to Towers:
typedef struct {
	byte lightLevel[N_COLORS]; // 0..255.  maps to analogWrite->light level
	byte fireLevel[N_COLORS]; // 0..255.  maps to timer->fire duration
} towerInstruction;

// EEPROM location for radio settings.
const byte radioConfigLocation = 42;
// default RFM group
#define D_GROUP_ID 188 

// defined nodes in the network.  
// Console
const byte consoleNodeID = 1;

// how many towers
#define N_TOWERS 4
// Towers
const byte towerNodeID[N_TOWERS] = {2,3,4,5};

// RFM12b comms
// Band  = 915 MHz
// Group ID = 188
// Simon 1-10
// Giles 11-20
// Clouds 21-210

#endif
