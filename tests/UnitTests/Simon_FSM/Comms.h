#ifndef Comms_h
#define Comms_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

// sizes, indexing and comms between Towers and Console
#include <EEPROM.h> // saving and loading radio settings
#include <RFM12B.h> // RFM12b radio transmitter module
#include <Simon_Comms.h>

// time before we reconfig the network
#define CONFIG_SET_INTERVAL 30000UL // ms
// when we send instructions, do so multiple times.
#define SEND_COUNT 3 // N
// but wait an interval of time before resending
#define SEND_INTERVAL 2UL // ms

// configure network
void configureNetwork();

// ping network for quality
void pingNetwork(int waitACK = D_WAIT_ACK);

// send configuration to Towers
void sendConfiguration();

// call this with update to tower light or fire.
void towerSend(int count=SEND_COUNT);

// all radio RX/TX should be handled here.  call periodically to TX/RX Towers.
void towerComms();


#endif
