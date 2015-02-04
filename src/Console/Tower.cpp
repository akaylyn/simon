#include "Tower.h"

// Need an instance of the Radio Module
RFM12B radio;

// time before we reconfig the network
#define CONFIG_SEND_INTERVAL 30000UL // ms.  check Tower module code for network timeout interval, and set this lower than that.
Metro networkConfigUpdate(CONFIG_SEND_INTERVAL);

// try to use these nodes:
//    to respond to a color (I_RED, I_GRN, I_BLU, I_YEL)
//    to respond to all colors (I_ALL)
//    to responde to nothing (ie. not used) (I_NONE)
byte towerColor[N_TOWERS] = {I_ALL, I_ALL, I_ALL, I_ALL};
byte towerFire[N_TOWERS] = {I_ALL, I_ALL, I_ALL, I_ALL};

// but wait an interval of time before resending
Metro resendCounter(SEND_INTERVAL);
// and countdown the remaining sends
int sendCount = 0;

// this is where the lights and fire instructions to Towers are placed
towerInstruction inst;
// from Simon_Comms.h:
// typedef struct {
//	byte lightLevel[N_COLORS]; // 0..255.  maps to analogWrite->light level
//	byte fireLevel[N_COLORS]; // 0..255.  maps to timer->fire duration
// } towerInstruction;
// structure stores how Towers acts on those towerInstructions
towerConfiguration config[N_TOWERS];

void towerStart() {
  Serial << F("Tower: startup.") << endl;

  // shouldn't need to run this once the memory is saved.
//  commsSave(consoleNodeID); // write to EEPROM. Select consoleNodeID.
  // start RFM12b radio
  if ( commsStart() != consoleNodeID  ) {
    Serial << F("Tower: unable to start RFM as consoleNodeID.  Halting!") << endl;
    while (1);
  }
  // ping the network
  pingNetwork();
  // configure network
  configureNetwork();
  // update network
  sendConfiguration();
}

// set lights on Tower with immediate send.  leaves previous instructions in-place!
void towerLightSet(byte colorIndex, byte level) {
  // set instructions
  inst.lightLevel[colorIndex] = level;
  // send it.
  towerSend();
}

// set fire on Tower with immediate send.  leaves previous instructions in-place!
void towerFireSet(byte colorIndex, byte level) {
  // set instructions
  inst.fireLevel[colorIndex] = level;
  // send it.
  towerSend();
}

// turns off light and fire on Tower with immediate send.
void towerQuiet() {
  // zero out
  commsDefault(inst);
  // send it.
  towerSend();
}

// sends the current value of inst to the Towers, and tries again in an interval by towerComms.
void towerSend(int sendN) {
  // send now
  commsSend(inst);
  // try again in an interval
  sendCount = sendN - 1;
  resendCounter.reset();
}

// check for inbound comms, resend instructions (if needed) periodically, send configuration periodically.
void towerUpdate() {

  // important that we check the buffer before sending, or we'll overwrite stuff
  if ( radio.ReceiveComplete() && radio.CRCPass() ) {
    // we have radio comms

    // check cases

    // join request.
    if ( radio.GetDataLen() == sizeof(towerConfiguration) ) {

      byte senderNodeID = radio.GetSender();
      Serial << F("Tower: join request from node ") << senderNodeID << endl;
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


// ping network for quality
void pingNetwork(int count) {
  Serial << F("Tower: ping network....") << endl;

  // check ping quality and use that to figure out how many Towers can be used.
  for ( byte ni = 0; ni < N_TOWERS; ni ++ ) {
    if ( towerColor[ni] != I_NONE || towerFire[ni] != I_NONE ) {
      Serial << F("Tower: ping node: ") << towerNodeID[ni]  << endl;

      // ping count times
      for ( int i = 0; i < count; i++ ) {
        commsSendPing(towerNodeID[ni]);
        Serial << F("+");
      }
      Serial << endl;
    }
  }
}

// configure network
void configureNetwork() {
  Serial << F("Tower: configuring network....") << endl;

  // check ping quality and use that to figure out how many Towers can be used.
  for ( byte ni = 0; ni < N_TOWERS; ni ++ ) {
    commsDefault(config[ni], towerColor[ni], towerFire[ni]);
    commsPrint(config[ni], towerNodeID[ni]);
  }
}

// send configuration to Towers
void sendConfiguration() {
  Serial << F("Tower: updating network....") << endl;

  // send the configuration to the network.
  // will be saved in EEPROM if new.
  for ( byte ni = 0; ni < N_TOWERS; ni ++ ) {
    Serial << F("Tower: sending configuration: ");
    commsPrint(config[ni], towerNodeID[ni]);
    commsSend(config[ni], towerNodeID[ni]);
  }

  // take a breather
  networkConfigUpdate.reset();

}



