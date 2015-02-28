// Responsible for UX (light/fire) output to Towers.  Coordinates outboard Tower.

#ifndef Tower_h
#define Tower_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers


//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h> 

// SPI library requirements: http://arduino.cc/en/Reference/SPI
#define RADIO_SCK 52 // SPI CLK
#define RADIO_SDO 50 // SPI MISO
#define RADIO_SDI 51 // SPI MOSI
#define RADIO_SEL 53 // SPI SS
#define RADIO_IRQ 2 // IRQ 0
#define D_CS_PIN 10 // default SS pin for RFM module

#include <SPI.h> // radio transmitter is a SPI device
#include <EEPROM.h> // saving and loading radio settings
#include <RFM12B.h> // RFM12b radio transmitter module

// startup Tower communications.
void towerStart();
  
// instantiates radio communications
byte networkStart();

// ping network for quality
#define D_PING_COUNT 10
void networkPing(int count=D_PING_COUNT);

// configure network
void networkConfig();

// configure a single tower
void towerConfig(towerConfiguration & config, byte nodeID);

// set lights on Tower with optional immediate send.  
void towerLightSet(byte colorIndex, byte level, boolean sendNow=false, byte nodeID=0);

// set fire on Tower with optional immediate send.  
void towerFireSet(byte fireIndex, byte level, boolean fireAllowed, boolean sendNow=false, byte nodeID=0);

// clears all of the instructions 
void towerClearInstructions();

// reissue gameplay packets this many times
#define TOWER_ECHO_COUNT 3

// turns off light and fire on Towers with immediate send.
void towerQuiet(int sendN=TOWER_ECHO_COUNT);

// sends the current value of inst to the Towers, and tries again in an interval by towerComms.
void towerSendAll(int sendN=TOWER_ECHO_COUNT);

// sends the current value of inst to a specific Tower, with no retries.
void towerSendOne(byte nodeID);

// check for inbound comms, resend instructions (if needed) periodically, send configuration periodically.
// but pause between this long
#define SEND_INTERVAL 5UL
void towerUpdate();

#endif
