// Network subunit.  Responsible for interfacing the Console with Towers and other projects

#ifndef Network_h
#define Network_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers.
// radio
#include <SPI.h> // radio transmitter is a SPI device
#include <EEPROM.h> // saving and loading radio settings
#include <RFM12B.h> // RFM12b radio transmitter module
// Light module
#include <EasyTransfer.h> 

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h> 

// once we get radio comms, wait this long  before returning false from externUpdate.
#define EXTERNAL_COMMS_TIMEOUT 10000UL

// SPI library requirements: http://arduino.cc/en/Reference/SPI
#define RADIO_SCK 52 // SPI CLK/SCK
#define RADIO_SDO 50 // SPI MISO/SDI
#define RADIO_SDI 51 // SPI MOSI/SDO
#define RADIO_SEL 53 // SPI SS/SEL
#define RADIO_IRQ 2 // IRQ 0
#define D_CS_PIN 10 // default SS pin for RFM module

class Network {
  public:
    void begin(color lightLayout[N_COLORS], color fireLayout[N_COLORS], nodeID node=BROADCAST); // defaults to getting nodeID from EEPROM
    void layout(color lightLayout[N_COLORS], color fireLayout[N_COLORS]); // update layout
    void update(); // should be called frequently for sync.
    
    // makes the network do stuff with your stuff
    void send(color position, colorInstruction &inst);
    void send(color position, fireInstruction &inst);
    void send(systemMode mode);
    
    void clear(); // clears all queued entries.
  
  private:
    // total system state, built up by public methods
    systemState state;
  
    // internal actuator of public send methods
    void send();
    
    // merges color and fire instructions when towers handle multiple channels
    void mergeColor(colorInstruction &inst);
    void mergeFire(fireInstruction &inst);

    // gets the network setup
    nodeID networkStart(nodeID node);
    nodeID node; // who am I, really?
    
    // send on an interval
    unsigned long packetSendInterval; // us
    
    // stores which towers should be sent color commands
    color lightLayout[N_COLORS];
    // stores which towers should be sent fire commands
    color fireLayout[N_COLORS];
    
    // Need an instance of the Radio Module.  
    RFM12B radio;
    
    // Need an instance of Easy Transfer
    EasyTransfer ET; 
};

extern Network network;

#endif
