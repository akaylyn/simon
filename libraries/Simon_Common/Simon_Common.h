#ifndef Simon_Common_h
#define Simon_Common_h

//**** Preamble
// we use serialized (bistream'd) structures to communicate, as these
// can be handled by either EasyTransfer (Serial*) and RFM69HW/RFM12b (radio)

//**** Radio

// Band  = 915 MHz
// Group ID = 188
#define D_GROUP_ID 188 
// radio nodes/adddresses/ID's
enum nodeID {
	BROADCAST=0, // _everyone_ on group 188
	CONSOLE=1,
	TOWER1=2,
	TOWER2=3,
	TOWER3=4,
	TOWER4=5
};
// Giles 11-20
// Clouds 21-210
// EEPROM location for radio setting storage.
const byte radioConfigLocation = 42;

//**** Fire

// solenoids take ~50ms to open fully.  Shorter requests are bumped up.
const unsigned long minPropaneTime = 50UL; 
// don't leave the solenoids open for longer than this interval.  Longer requests are bumped down.
const unsigned long maxPropaneTime = 2000UL; 
// after opening the flame solenoid for N ms, don't reopen for N * propaneClosedMultiplier ms.
const unsigned int propaneClosedMultiplier = 1;

// can add air to the propane for different effects
// by testing, we want to pulse the air 
const unsigned long airPulseTime = 50UL;
// by testing, we want to delay introduction of the air
const unsigned long delayAirTime = 50UL;

enum flameEffect {
	// no air.  just straight propane.
	veryRich=0, // "very rich"
	
	// add some air to 1/3rd of the flame time; not notably different for small (<100ms) flames
	kickStart,  // toss in some air at the beginning
	kickMiddle,  // toss in some air in the middle
	kickEnd,  // toss in some air at the end
	
	// add air throughout flame time
	gatlingGun, // staccato bursts of air throughout
	randomly, // air tossed in throughout in a random pattern
	veryLean, // as much air as as we can before getting "too lean"
	
	N_flameEffects  // use this to size arrays appropriately
};

typedef struct {
	byte flame; // flame duration in 10's of ms. e.g. "42" maps to 420 ms.  See min and max constraints.
	byte effect; // see above
} fireInstruction;

// and some definitions, so we're all on the same page
const fireInstruction littlePoof = {minPropaneTime/10, veryRich};
const fireInstruction ragingInferno = {maxPropaneTime/10, randomly};

//***** Colors

// relative locations for the colors on the Simon Console
enum color {
	I_RED=0, // upper right
	I_GRN,   // upper left
	I_BLU,   // lower right
	I_YEL,   // lower left
	
	N_COLORS // use this to size arrays appropriately
};

typedef struct {
	byte red;
	byte green;
	byte blue;
} colorInstruction;

// and some definitions, so we're all on the same page
const colorInstruction cRed = {255, 0, 0};
const colorInstruction cGreen = {0, 255, 0};
const colorInstruction cBlue = {0, 0, 255};
const colorInstruction cYellow = {255, 255, 0};
const colorInstruction cWhite = {255, 255, 255};
// and this serves as an easy way to pull out the right RGB color from the 
const colorInstruction cMap[N_COLORS] = {cRed, cGreen, cBlue, cYellow};

//**** System Modes

// How many different gameplay/test modes we have
#define NUM_MODES 5

// Send to the towers to tell them what mode they're in when we switch modes.
typedef struct {
  byte currentMode;
} modeSwitchInstruction;

//**** Communcation Check

typedef struct {
	int packetNumber;
	int packetTotal;
} commsCheckInstruction;

//**** Sanity Checks

// we rely on being able to differentiate different instructions by their size.
// better make sure they're a different size

#define N_DATAGRAMS 4
const byte instructionSizes[N_DATAGRAMS] = {
	sizeof(fireInstruction), 
	sizeof(colorInstruction), 
	sizeof(modeSwitchInstruction), 
	sizeof(commsCheckInstruction)
};

//**** Lights on the Console

// handle lighting instructions sent to the Light modile
// to be clear: this type of instruction is not sent over the radio.

// enumerate all of the different animations that Light can perform
enum animationInstruction {
	SOLID=0,
	
	N_animations // ==1
};

enum lightUnit {
	lRed=0, // strips 
	lGrn,
	lBlue,
	lYellow, // the four big buttons

	N_lightUnits // == 4
};

typedef struct {
	colorInstruction color[N_lightUnits];
	animationInstruction anim[N_lightUnits];
} lightModuleInstruction;

#endif
