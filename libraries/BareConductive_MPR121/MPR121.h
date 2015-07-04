/*******************************************************************************

 Bare Conductive MPR121 library
 ------------------------------

 MPR121.h - MPR121 class header file

 Based on code by Jim Lindblom and plenty of inspiration from the Freescale
 Semiconductor datasheets and application notes.

 Bare Conductive code written by Stefan Dzisiewski-Smith and Peter Krige.

 This work is licensed under a Creative Commons Attribution-ShareAlike 3.0
 Unported License (CC BY-SA 3.0) http://creativecommons.org/licenses/by-sa/3.0/

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

*******************************************************************************/

#ifndef MPR121_H
#define MPR121_H

#include "MPR121_defs.h"
#include <Wire.h>

// idea behind this is to create a settings structure that we can use to store
// all the setup variables for a particular setup - comes pre-instantiated with
// defaults and can be easily tweaked - we pass by reference (as a pointer) to
// save RAM

struct MPR121_settings_t
{
	// touch and release thresholds
	unsigned char TTHRESH;
	unsigned char RTHRESH;

	unsigned char INTERRUPT;

	// general electrode touch sense baseline filters
	// rising filter
	unsigned char MHDR;
	unsigned char NHDR;
	unsigned char NCLR;
	unsigned char FDLR;

	// falling filter
	unsigned char MHDF;
	unsigned char NHDF;
	unsigned char NCLF;
	unsigned char FDLF;

	// touched filter
	unsigned char NHDT;
	unsigned char NCLT;
	unsigned char FDLT;

	// proximity electrode touch sense baseline filters
	// rising filter
	unsigned char MHDPROXR;
	unsigned char NHDPROXR;
	unsigned char NCLPROXR;
	unsigned char FDLPROXR;

	// falling filter
	unsigned char MHDPROXF;
	unsigned char NHDPROXF;
	unsigned char NCLPROXF;
	unsigned char FDLPROXF;

	// touched filter
	unsigned char NHDPROXT;
	unsigned char NCLPROXT;
	unsigned char FDLPROXT;

	// debounce settings
	unsigned char DTR;

	// configuration registers
	unsigned char AFE1;
	unsigned char AFE2;
	unsigned char ECR;

	// auto-configuration registers
	unsigned char ACCR0;
	unsigned char ACCR1;
	unsigned char USL;
	unsigned char LSL;
	unsigned char TL;

	// default values in initialisation list
	MPR121_settings_t():
		TTHRESH(40),
		RTHRESH(19),
		INTERRUPT(4), 	// note that this is not a hardware interrupt, just the digital
						// pin that the MPR121 ~INT pin is connected to

		// MGD: baseline filtering.  Section 5.5 of data sheet.
		// see AN3891.pdf for a detailed discussion.
		// MGD these control baseline filtering operations during rising values
		// It appears that the baseline can drift up very quickly.
		MHDR(0x3F), // 63.  very large.
		NHDR(0x3F), // 63.  very large.
		NCLR(0x05), // 5.
		FDLR(0x00), // 0.
		// MGD these control baseline filtering operations during falling values
		MHDF(0x01), // 1.  very small.
		NHDF(0x3F), // 63. very large.
		NCLF(0x10), // 16. 
		FDLF(0x03), // 3.
		// MGD these control baseline filtering operations during Touched.
		// no maximum half-delta in Touched mode.
		// does appear to allow changes, but over a very long interval.
		NHDT(0x01), // 1. very small.
		NCLT(0x01), // 1. very small

// MGD: NEED TO LET THE BASELINE DRIFT DURING TOUCHED MODE
// SEE AN3891.pdf

// this setting prevents any drift in the baseline value
//		FDLT(0xFF), // 255.  huge.

		// MGD tinkering to allow the baseline to drift during Touched mode more
// this setting is too fast.
		FDLT(0x01), // real fast change.  a really held button registers release in ~7s.

// this feels pretty good, but still a fast change.  Too fast, and a release will be prematurely triggered.
//		FDLT(0x0F), // slower change.  

		// MGD: these are the same thing, but for Proximity mode, which we're 
		// not using.  Ignore.
		MHDPROXR(0x0F),
		NHDPROXR(0x0F),
		NCLPROXR(0x00),
		FDLPROXR(0x00),
		MHDPROXF(0x01),
		NHDPROXF(0x01),
		NCLPROXF(0xFF),
		FDLPROXF(0xFF),
		NHDPROXT(0x00),
		NCLPROXT(0x00),
		FDLPROXT(0x00),

		DTR(0x11),
		AFE1(0xFF),
		AFE2(0x38),
		//ECR(0x8C), // default to fast baseline startup and 12 electrodes enabled, no prox
		//ECR(0x84), // only enable pins 0-3
		//ECR(0xC4), // CL:11 baseline track, PROX:00 disabled, ELEC 0-3
		//ECR(0x44), // CL:01 baseline track off, PROX:00 disabled, ELEC 0-3
                ECR(0xC4), // CL:11 baseline track, PROX:00 disabled, ELEC 0-3



        /**********************************************************************
         * Auto Calibration Configuration
         *
         * does enabling only the pins in use make the autoconfig better/faster?
         * it may or may not more accurately read touches through plastic
         * it seems like it is better.
         *
         * adding copper tape to the ends of the pins provides enough sensitivity
         * for detecting touches onto the foil (and through plastic)
         *********************************************************************/
        /* ACCR0 - Auto Configure Control Register
         *  see the data sheet, page 17
         *  FFI = 11 (same as AFE1.FFI, 0x5C)
         *  BVA = 11 (same as ECR.CL, 0x5E)
         */
//		ACCR0(0xFA), // FFI:11, RETRY:11, BVA:10, ARE:1, ACE:0 // autoconfig off!
//		ACCR0(0xFB), // FFI:11, RETRY:11, BVA:10, ARE:1, ACE:1 // autoconfig on.
		ACCR0(0xFF), // FFI:11, RETRY:11, BVA:11, ARE:1, ACE:1 
		// NOTE: BVA in ACCR0 must match CL in ECR

		ACCR1(0x00), // SCTS: 0, OORIE:0, ARFIE:0, ACFIE:0

		USL(0xC9), // upper side limit= 201<<2
		LSL(0x82), // lower side limit= 130<<2
		TL(0xB5) {} // target level= 181<<2

        // disabled mode
        /*
		ACCR0(0x00),
		ACCR1(0x00),
		USL(0x00),
		LSL(0x00),
		TL(0x00) {}
        */

        /*
        // Auto config options calibrated for 3.3V
        // Section G - Set Auto Config and Auto Reconfig for prox sensing
        set_register(0x5A, ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V
        set_register(0x5A, ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
        set_register(0x5A, ATO_CFGT, 0xB5);  // Target = 0.9*USL = 0xB5 @3.3V
        set_register(0x5A, ATO_CFG0, 0x0B);
        */

};

// GPIO pin function constants
enum mpr121_pinf_t
{
	// INPUT and OUTPUT are already defined by Arduino, use its definitions

	//INPUT,		// digital input
	INPUT_PU,		// digital input with pullup
	INPUT_PD,		// digital input with pulldown
	//OUTPUT,		// digital output (push-pull)
	OUTPUT_HS,		// digital output, open collector (high side)
	OUTPUT_LS		// digital output, open collector (low side)
};

// "13th electrode" proximity modes
// N.B. this does not relate to normal proximity detection
// see http://cache.freescale.com/files/sensors/doc/app_note/AN3893.pdf
enum mpr121_proxmode_t
{
	DISABLED,		// proximity mode disabled
	PROX0_1,		// proximity mode for ELE0..ELE1
	PROX0_3,		// proximity mode for ELE0..ELE3
	PROX0_11		// proximity mode for ELE0..ELE11
};

// error codes
enum mpr121_error_t
{
	NO_ERROR,			// no error
	RETURN_TO_SENDER,	// not implemented
	ADDRESS_UNKNOWN,	// no MPR121 found at specified I2C address
	READBACK_FAIL,		// readback from MPR121 was not as expected
	OVERCURRENT_FLAG,	// overcurrent on REXT pin
	OUT_OF_RANGE,		// autoconfiguration fail, often a result of shorted pins
	NOT_INITED			// device has not been initialised
};

class MPR121_t
{
	private:
		// internal helper functions and variables
		// not exposed externally to the user
		unsigned char address;
		MPR121_settings_t defaultSettings;
		unsigned char ECR_backup; // so we can re-enable the correct number of electrodes
								  // when recovering from stop mode
		unsigned char error;
		bool running;
		int interruptPin;

		int filteredData[13];
		int baselineData[13];
		unsigned int touchData;
		unsigned int lastTouchData;
		bool getLastTouchData(unsigned char electrode);

	public:
		MPR121_t();

		// -------------------- BASIC FUNCTIONS --------------------

		// begin() must be called before using any other function
		// address is optional, default is 0x5C
		bool begin(unsigned char address);
		bool begin();

		// getError() returns an mpr121_error_t indicating the current
		// error on the MPR121 - clearError() clears this
		mpr121_error_t getError();
		void clearError();

		// returns status of the MPR121 INT pin as read via digitalRead() on the
		// Arduino board - this tells us if there has been a change in touch status
		// on any active electrode since we last read any data
		bool touchStatusChanged();

		// updates the data from the MPR121 into our internal buffer
		// updateTouchData() does this only for touch on / off status
		// updateBaseLineData() does this for background baseline
		// updateFilteredData() does this for continuous proximity data
		// updateAll() does all three

		// the appropriate function from these must be called before data
		// from getTouchData(), getFilteredData() etc. can be considered
		// valid
		void updateTouchData();
		bool updateBaselineData();
		bool updateFilteredData();
		void updateAll();

		// returns a boolean indicating the touch status of a given electrode
		bool getTouchData(unsigned char electrode);

		// returns the number of touches currently detected
		unsigned char getNumTouches();

		// returns continous proximity or baseline data for a given electrode
		int getFilteredData(unsigned char electrode);
		int getBaselineData(unsigned char electrode);

		// returns boolean indicating whether a new touch or release has been
		// detected since the last time updateTouchData() was called
		bool isNewTouch(unsigned char electrode);
		bool isNewRelease(unsigned char electrode);

		// sets touch and release thresholds either for all electrodes, or
		// for a specfic electrode - higher values = less sensitive and
		// release threshold must ALWAYS be lower than touch threshold
		void setTouchThreshold(unsigned char val);
		void setTouchThreshold(unsigned char electrode, unsigned char val);
		void setReleaseThreshold(unsigned char val);
		void setReleaseThreshold(unsigned char electrode, unsigned char val);

		// returns the current touch or release threshold for a specified electrode
		unsigned char getTouchThreshold(unsigned char electrode);
		unsigned char getReleaseThreshold(unsigned char electrode);

		// ------------------ ADVANCED FUNCTIONS ------------------

		// applies a complete array of settings from an
		// MPR121_settings_t variable passed as a pointer
		// useful if you want to do a bulk setup of the device
		void applySettings(MPR121_settings_t *settings);

		// setRegister() and getRegister() manipulate registers on
		// the MPR121 directly, whilst correctly stopping and
		// restarting the MPR121 if necessary
		void setRegister(unsigned char reg, unsigned char value);
		unsigned char getRegister(unsigned char reg);

		// stop() and run() take the MPR121 in and out of stop mode
		// which reduces current consumption to 3uA
		void run();
		void stop();

		// resets the MPR121
		bool reset();

		// tells us if we are in run mode, and if we have inited the
		// MPR121
		bool isRunning();
		bool isInited();

		// sets the pin that the MPR121 INT output is connected to on the
		// Arduino board - does not have to be a hardware interrupt pin
		void setInterruptPin(unsigned char pin);

		// set number of electrodes to use to generate virtual "13th"
		// proximity electrode
		// see http://cache.freescale.com/files/sensors/doc/app_note/AN3893.pdf
		//
		// N.B. - this is not related to general proximity detection or
		// reading back continuous proximity data
		void setProxMode(mpr121_proxmode_t mode);

		// Enables GPIO mode for up to 8 of the MPR121 electrodes
		// starts with electrode 11 - i.e. setNumDigPins(1) sets just
		// electrode 11 as GPIO, setNumDigPins(2) sets electrodes 11
		// & 10 as GPIO, and so on. Electrodes 0 to 3 cannot be used
		// as GPIO
		//
		// N.B. electrodes are 3.3V and WILL be damaged if driven by
		// a greater voltage
		void setNumDigPins(unsigned char numPins);

		// Sets pin mode for an electrode already set as GPIO by
		// setNumDigPins() - see section "GPIO pin function constants"
		// for details
		void pinMode(unsigned char electrode, mpr121_pinf_t mode);
		void pinMode(unsigned char electrode, int mode);

		// Similar to digitalWrite in Arduino for GPIO electrode
		void digitalWrite(unsigned char electrode, unsigned char val);

		// Toggles electrode set as GPIO output
		void digitalToggle(unsigned char electrode);

		// Reads electrode set as GPIO input
		bool digitalRead(unsigned char electrode);

		// Writes PWM value to electrode set as GPIO output - very limited
		// (4 bit, although input to function is 0..255 to match Arduino,
		// internally reduced to 4 bit) and broken on ELE9 and ELE10
		// see https://community.freescale.com/thread/305474
		void analogWrite(unsigned char electrode, unsigned char val);

};

extern MPR121_t MPR121;

#endif // MPR121_H
