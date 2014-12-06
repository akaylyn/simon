#include "Comms.h"

// instantiated from main codes
//extern RFM12B radio;

// Need an instance of the Radio Module
RFM12B radio;

// time before we reconfig the network
Metro networkConfigUpdate(CONFIG_SET_INTERVAL);

// try to use these nodes:
//    to respond to a color (I_RED, I_GRN, I_BLU, I_YEL)
//    to respond to all colors (I_ALL)
//    to responde to nothing (ie. not used) (I_NONE)
byte towerColor[N_TOWERS] = {I_ALL, I_NONE, I_NONE, I_NONE};
byte towerFire[N_TOWERS] = {I_ALL, I_NONE, I_NONE, I_NONE};

// but wait an interval of time before resending
Metro resendCounter(SEND_INTERVAL);
// and countdown the remaining sends
int sendCount = 0;

// this is where the lights and fire instructions to Towers are placed
extern towerInstruction inst;
// structure stores how Towers acts on those towerInstructions
towerConfiguration config[N_TOWERS];

// configure network
void configureNetwork() {
  Serial << F("Configuring network....") << endl;

  // check ping quality and use that to figure out how many Towers can be used.
  for ( byte ni = 0; ni < N_TOWERS; ni ++ ) {
    commsDefault(config[ni], towerColor[ni], towerFire[ni]);
    commsPrint(config[ni], towerNodeID[ni]);
  }
}

// ping network for quality
void pingNetwork(int waitACK) {
  Serial << F("Ping network....") << endl;

  // check ping quality and use that to figure out how many Towers can be used.
  for ( byte ni = 0; ni < N_TOWERS; ni ++ ) {
    if ( towerColor[ni] != I_NONE || towerFire[ni] != I_NONE ) {
      Serial << F("Ping node: ") << towerNodeID[ni] << endl;

      // store the number of ACKs we get back
      int ACKcount = 0;
      // ping 100 times
      for ( int i = 0; i < 100; i++ ) {
        if ( commsSendPing(towerNodeID[ni], waitACK) ) {
          ACKcount++;
          Serial << F("+");
        } else {
          Serial << F("_");
        }
      }
      Serial << endl;

      Serial << F("Quality = ") << ACKcount << F("%.") << endl;
    }
  }
}


// send configuration to Towers
void sendConfiguration() {
  Serial << F("Updating network....") << endl;

  // send the configuration to the network.
  // will be saved in EEPROM if new.
  for ( byte ni = 0; ni < N_TOWERS; ni ++ ) {
    Serial << "Sending configuration: ";
    commsPrint(config[ni], towerNodeID[ni]);
    commsSend(config[ni], towerNodeID[ni]);
  }

  // take a breather
  networkConfigUpdate.reset();

}

// sends the current value of inst to the Towers, and tries again in an interval by towerComms.
void towerSend(int sendN) {
  sendCount = sendN;
}

// all radio RX/TX should be handled here.
void towerComms() {

  // important that we check the buffer before sending, or we'll overwrite stuff
  if ( radio.ReceiveComplete() && radio.CRCPass() ) {
    // we have radio comms
    
    // check cases
    
    // join request.
    if ( radio.GetDataLen() == sizeof(towerConfiguration) ) {
      
      byte senderNodeID = radio.GetSender();
      Serial << F("Join request from node ") << senderNodeID << endl;
      sendConfiguration();  // send network configuration settings
    }

  }

  // then we process outbound traffic
  if ( sendCount > 0 && resendCounter.check() ) { // we need to send the instructions
    sendCount--; // one less time
    resendCounter.reset();

    commsSend(inst);
  } else if ( networkConfigUpdate.check() ) { // update network
    sendConfiguration();
  }

}




