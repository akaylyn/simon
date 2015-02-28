// Compile for UNO

#include <SPI.h> // SPI internal for APA102s
#include <Adafruit_DotStar.h> // APA102 support; https://github.com/adafruit/Adafruit_DotStar/archive/master.zip
// NOTE: I tried fastLED 3.0.2 for APA102 support, which worked, but it failed utterly for WS2812Bs.
#include <Adafruit_NeoPixel.h> // WS2812B support;
#include <RFM12B.h> // RFM12b/69 compatibility; https://github.com/aanon4/RFM12B
#include <EEPROM.h> // baby hard disk; included in Arduino IDE
#include <Streaming.h> // <<-style printing; http://arduiniana.org/Streaming/Streaming5.zip
#include <Metro.h> // timers; https://github.com/thomasfredericks/Metro-Arduino-Wiring

/* Generally, we'll run this test with two Nodes.

Role==Tx.
  * saves the role to EEPROM for later retrieval.
  * spams packets on an interval with a packet counter.

Role==Rx.
  * saves the role to EEPROM for later retreival.
  * updates test pattern on LED array.
  * reads the packets and reports packet loss.

Role==getFromEEPROM.
  * goes to EEPROM to select Tx/Rx role.
  * once the Role is set once, this should be the default operating mode.
*/
enum Role { getFromEEPROM = 0, Tx = 1, Rx = 2 };
//Role myRole = Tx; // run once on Tx node.
//Role myRole = Rx; // run once on Rx node.
Role myRole = getFromEEPROM; // run on either node, and EEPROM ref'd to get Role.

// Need an instance of the Radio Module
RFM12B radio;
#define R_GROUP 100
#define R_BAND RF12_915MHZ

// packet we'll be sending from Tx:
#define PAD_LENGTH 5
typedef struct {
  unsigned long packetCount;
  byte padding[PAD_LENGTH]; // padding added to flunk CRC if a bit's dropped.
} radioPacket;
radioPacket packet;
// how many packets do we want to send?
#define N_PACKETS 100

// APA102 led strip.  Note, this is only used on the Rx side.
#define APA_NLED 200
#define APA_DATA_PIN 4 // DI. JeeNode DO on Port 1. at the end of JeeNode LN.
#define APA_CLOCK_PIN 7 // CI.  JeeNode DO on Port 4. mid-span of JeeNode LN.
Adafruit_DotStar APAleds = Adafruit_DotStar(APA_NLED, APA_DATA_PIN, APA_CLOCK_PIN);
const uint32_t aRed = APAleds.Color(255, 0, 0);
const uint32_t aYel = APAleds.Color(255, 255, 0);
const uint32_t aGrn = APAleds.Color(0, 255, 0);
const uint32_t aBlu = APAleds.Color(0, 0, 255);
const uint32_t aBlack = APAleds.Color(0, 0, 0);

// WS2812B led strip.  Note, this is only used on the Rx side.
#define WS_NLED 200
#define WS_DATA_PIN A0 // AI. JeeNode AO on Port 4. at the end of JeeNode LN.
Adafruit_NeoPixel WSleds = Adafruit_NeoPixel(WS_NLED, WS_DATA_PIN, NEO_GRB + NEO_KHZ800);
const uint32_t wRed = WSleds.Color(255, 0, 0);
const uint32_t wYel = WSleds.Color(255, 255, 0);
const uint32_t wGrn = WSleds.Color(0, 255, 0);
const uint32_t wBlu = WSleds.Color(0, 0, 255);
const uint32_t wBlack = WSleds.Color(0, 0, 0);

void setup() {
  // startup Serial debug
  Serial.begin(115200);

  // random seed.
  randomSeed(analogRead(A3)); // or some other unconected pin

  // figure out our role
  getRole();

  // start up the radio, using Role as the nodeID.
  radio.Initialize(myRole, R_BAND, R_GROUP);
  Serial << F("Radio setup.") << endl;

  // start up APAleds
  if ( myRole == Rx ) {
    APAleds.begin();
    APAleds.show();
    Serial << F("APA LED strip setup.") << endl;

    WSleds.begin();
    WSleds.show();
    Serial << F("WS LED strip setup.") << endl;

  }

}

void loop() {
  // by role, do stuff
  switch ( myRole ) {
    case Tx:
      
      doTx(); // send radio traffic
      
      break;
    case Rx:
      
      doRx(); // receive radio traffic
      
      static boolean showAPA=true;
      static boolean showWS=false;
      doLeds(showAPA, showWS); // update the leds
      
      break;
    default:
      
      Serial << F("Role not configured.  Halting.") << endl;
      while (1);
  }
}

// spamming the airwaves with packets.
Metro txCountdown(3UL * 1000UL);   // units of ms.  wait this long between sends.
void doTx() {
  // if it's not time to send, return.
  if ( !txCountdown.check() ) return;

  Serial << F("Starting packet send at ") << millis() / 1000 << F(" sec.") << endl;

  // otherwise, reset packet.
  for ( int npad = 0; npad < PAD_LENGTH; npad++ )
    packet.padding[npad] = random(0, 255 + 1);

  // units of ms.  wait this long between packets.
  unsigned long sendInterval = 10;
  for ( unsigned long npack = 0; npack < N_PACKETS; npack++ ) {
    // store the packet count
    packet.packetCount = npack;

    // make sure the transmitter is ready to send.  So, can push the sendInterval to 0, and this would
    // be upper bound on Tx speed (by baudrate in RFM setup).
//    while ( !radio.CanSend() ) radio.ReceiveComplete();

    // send now, broadcast mode.
    radio.Send(0, (const void*)(&packet), sizeof(packet));
    radio.SendWait(); // wait for completion
    
    // wait for next send.
 //   delay(sendInterval);
  }

  Serial << F("Packet sending complete at ") << millis() / 1000 << F(" sec.") << endl;

  // ready for next round.
  txCountdown.reset();
}

Metro resetPacketCount(1000UL);   // after radio traffic has halted for this long, zero out the packet count
// listen for packets and track percent receieved.
void doRx() {

  // track packet count received.
  static unsigned long packetsRx = 0;

  if ( radio.ReceiveComplete() && radio.CRCPass() ) {
    // we got traffic
    if ( radio.GetDataLen() == sizeof(radioPacket) ) {
      // and it's the right size
      packetsRx++;

      // save it.  Not used, but we would in production.
      packet = *(radioPacket*)radio.GetData();

      // reset counter.
      resetPacketCount.reset();
    }
  }

  if ( resetPacketCount.check() && packetsRx > 0 ) {
    Serial << F("Packet Rx complete.") << endl;
    Serial << F("Received intact = ") << packetsRx;
    Serial << F(" expected = ") << N_PACKETS;
    Serial << F(". Percent intact = ") << (packetsRx * 100UL) / N_PACKETS << endl;
    // reset for next loop
    packetsRx = 0;
  }
}

// use a red color scheme, as any flicker (blue or green) is readily notable.
void doLeds(boolean pushAPA, boolean pushWS) {

  // otherwise, show how long we've been operating in binary on the first 32 APAleds, in red.
  unsigned long now = millis();
  for( int b=0; b<32; b++ ) {
    APAleds.setPixelColor(b, bitRead(now, b) ? aRed : aBlack);
    WSleds.setPixelColor(b, bitRead(now, b) ? wRed : wBlack);
  }
  
  // send it.
  if( pushAPA) APAleds.show();
  if( pushWS ) WSleds.show();
  
}

// gets or sets role from or to EEPROM
void getRole() {
  // EEPROM location for role setting
  const byte roleLocation = 118;

  switch ( myRole ) {
    case getFromEEPROM:
      myRole = EEPROM.read(roleLocation) == 1 ? Tx : Rx;
      Serial << F("Read role from EEPROM.") << endl;
      break;
    default: // manually setting Rx or Tx writes that choice to EEPROM.  Switch to getFromEEPROM role after.
      EEPROM.write(roleLocation, myRole);
      Serial << F("Wrote role to EEPROM.") << endl;
      break;
  }
  
  Serial << F("Role is: ") << myRole << endl;
}
