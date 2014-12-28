// Responsible for UX (light/fire) output to Towers.  Coordinates outboard Tower.

#ifndef Tower_h
#define Tower_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

// RFM12b radio:
/* JeeLibs RFM12b radio comms:
  See: http://jeelabs.org/2011/02/02/meet-the-rfm12b-board/
  
  +5V -> 5V
  Ground -> GND
  SPI clock (SCK) –> Uno digital 13 (Mega 52)
  SPI data out (SDO) –> Uno digital 12 (Mega 50)
  SPI data in (SDI) –> Uno digital 11 (Mega 51)
  SPI select (SEL) –> Uno digital 10
  IRQ –> Uno digital 2
  +3.3V -> (unused, but a regulated 3.3 source if we need it)
*/
// SPI library requirements: http://arduino.cc/en/Reference/SPI
#define RADIO_SCK 52 // SPI CLK
#define RADIO_SDO 50 // SPI MISO
#define RADIO_SDI 51 // SPI MOSI
#define RADIO_SEL 53 // SPI SS
#define RADIO_IRQ 2 // IRQ 0

#include <Simon_Indexes.h> // sizes, indexing
#include <Simon_Comms.h> // comms between Towers and Console
#include <RFM12B.h> // RFM12b radio transmitter module
#include <SPI.h> // radio transmitter is a SPI device
#include <EEPROM.h> // saving and loading radio settings

// reissue gameplay packets this many times
#define TOWER_ECHO_COUNT 3
// but pause between this long
#define SEND_INTERVAL 5UL

// tower startup
void towerStart();

// set lights on Tower with immediate send.  leaves previous instructions in-place!
void towerLightSet(byte colorIndex, byte level);

// set fire on Tower with immediate send.  leaves previous instructions in-place!
void towerFireSet(byte colorIndex, byte level);

// turns off light and fire on Tower with immediate send.
void towerQuiet();

// sends the current value of inst to the Towers, and tries again in an interval by towerUpdate.
void towerSend(int sendN=TOWER_ECHO_COUNT);

// check for inbound comms, resend instructions (if needed) periodically, send configuration periodically.
void towerUpdate();

// ping network for quality
void pingNetwork(int waitACK=D_WAIT_ACK);

// configure network
void configureNetwork();

// send configuration to Towers
void sendConfiguration();

#endif
