// Instructions subunit.  Gets radio traffic and handles idle patterns.

#ifndef Instruction_h
#define Instruction_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module
#include <EEPROM.h> // saving and loading radio settings

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h> 

class Instruction {
  public:
    Instruction(byte setID);
    boolean update(towerInstruction &inst);
    
  protected:   
    // Need an instance of the Radio Module
    RFM69 radio;
    // store my NODEID
    byte myNodeID;
    // gets the network setup
    void networkStart(byte nodeID);
};


#endif

