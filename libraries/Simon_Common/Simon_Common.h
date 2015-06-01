#ifndef Simon_Common_h
#define Simon_Common_h

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

// How many different gameplay/test modes we have
#define NUM_MODES 5

// nodeID=0 is understood to be "everyone" (broadcast).

// RFM12b comms
// Band  = 915 MHz
// Group ID = 188
// Simon 1-10
// Giles 11-20
// Clouds 21-210

// how many colors
#define N_COLORS 4
// specific color indices to arrays
#define I_RED 0
#define I_GRN 1
#define I_BLU 2
#define I_YEL 3

// solenoids take ~50ms to open fully.  Shorter requests are bumped up.
const unsigned long minPropaneTime = 50UL; 
// don't leave the solenoids open for longer than these intervals.  Longer requests are bumped down.
const unsigned long maxPropaneTime = 2000UL; 
// after opening the flame solenoid for N ms, don't reopen for N * ?ClosedMultiplier ms.
const unsigned int propaneClosedMultiplier = 1;

// can add air to the propane for different effects
// by testing, we want to pulse the air 
const unsigned long airPulseTime = 50UL;
// by testing, we want to delay introduction of the air
const unsigned long delayAirTime = 50UL;

enum flameEffect_t {
	// no air.  just straight propane.
	FE_veryRich, // "very rich"
	
	// add some air to 1/3rd of the flame time; not notably different for small (<100ms) flames
	FE_kickStart,  // toss in some air at the beginning
	FE_kickMiddle,  // toss in some air in the middle
	FE_kickEnd,  // toss in some air at the end
	
	// add air throughout flame time
	FE_gatlingGun, // staccato bursts of air throughout
	FE_randomly, // air tossed in throughout in a random pattern
	FE_veryLean, // as much air as as we can before getting "too lean"
};

// during gameplay, this is the information passed from Console to Towers to control lights and fire:
typedef struct {
	byte red; // red level
	byte green; // green level
	byte blue; // blue level
	byte flame; // flame duration in 10's of ms.
	flameEffect_t effect; // see above
} towerInstruction;

// Send to the towers to tell them what mode they're in when we switch modes.
typedef struct {
  byte currentMode;
} modeSwitchInstruction;


// predefined Tower configuration settings
enum towerLightFireConfig_t { EACH2OWN, ALLIN };
// EACH2OWN: each tower listens to a single color (and related fire) channel
// ALLIN: all towers listen to all color (and related fire) channels

// during operation, the Console can be reconfigured on-the-fly
typedef struct {
	// how should Towers respond to light commands?
	towerLightFireConfig_t towerLightResponse;
	// how should Towers respond to fire commands?
	towerLightFireConfig_t towerFireResponse;

	// how long (ms) between fanfare displays when the system has been idle?
	unsigned long kioskFanfareInterval; 

	// should gameplay let you win with any button press?
	boolean cheatyPantsMode;
	// how many correct moves does Player1 need for each fanfare level?
	byte correctsForFanfare[4];

	// sound levels.  Master [-70 to +4db]
	int masterGain;
	// Tone gain relative to Master
	int toneGainRelativeToMaster;
	// Track gain relative to Tone;
	int trackGainRelativeToTone;
} consoleConfiguration;
// EEPROM location for consoleConfiguration settings.
const byte consoleConfigLocation = 180;

// during operation, the Console can be asked to perform tasks.
// Console will also answer to towerInstructions with the implied setting lightListen={true, true, true, true}.
typedef struct {
	// play some music [1,999].  Set to zero to get a random WINS song; set to -1 to have no Music.
	int playThisTrack;

	// if you ask for Music, do you want the Console to compose light and fire to accompany?
	// if you say "yes"/true, then the Console will send towerInstructions so you shouldn't.
	boolean lightsAndFireWithMusic;

	// do you want button tones to accompany towerInstructions that you send?
	boolean tonesWithTowerInstructions;
} consoleInstruction;

#endif
