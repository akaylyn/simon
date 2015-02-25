// Compile for UNO

#include <FastLED.h> // APA102 support; https://github.com/FastLED/FastLED
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
// Role myRole = Tx; // run once on Tx node.
// Role myRole = Rx; // run once on Rx node.
Role myRole = getFromEEPROM; // run on either node, and EEPROM ref'd to get Role.

// Need an instance of the Radio Module
RFM12B radio;
#define R_GROUP 100
#define R_BAND RF12_915MHZ

// packet we'll be sending from Tx:
#define PAD_LENGTH 10
typedef struct {
  unsigned long packetCount;
  byte padding[PAD_LENGTH]; // padding added to flunk CRC if a bit's dropped.
} radioPacket;
radioPacket packet;
// how many packets do we want to send?
#define N_PACKETS 100

// led strip.  Note, this is only used on the Rx side.
#define NUM_LEDS 66
// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];
// hardware SPI is "taken" by the radio, so let's offload that to different pins
#define DATA_PIN 4 // DI. JeeNode DO on Port 1.
#define CLOCK_PIN 7 // CI.  JeeNode DO on Port 4.
#define DATA_RATE 12 // units of MHz.  24 MHz is the default, but that's silly.
  // Note: I wouldn't choose 915 MHz, either, since there's a radio nearby transmitting at that frequency.
#define COLOR_ORDER GRB

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

  // start up LEDs
  if ( myRole == Rx ) {
    FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(DATA_RATE)>(leds, NUM_LEDS);
    byte maxBrightness = 64; // less dazzling for the operator
    FastLED.setBrightness(maxBrightness);
    Serial << F("LED strip setup.") << endl;
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
      doLEDs(); // update the LEDs
      break;
    default:
      Serial << F("Role not configured.  Halting.") << endl;
      while (1);
  }
}

// spamming the airwaves with packets.
void doTx() {
  // units of ms.  wait this long between sends.
  static Metro txCountdown(10UL * 1000UL);
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
    while ( !radio.CanSend() );

    // send now, broadcast mode.
    radio.Send(0, (const void*)(&packet), sizeof(packet));

    // wait for next send.
    delay(sendInterval);
  }

  // ready for next round.
  txCountdown.reset();
}

// listen for packets and track percent receieved.
void doRx() {
  // after radio traffic has halted for this long, zero out the packet count
  static Metro resetPacketCount(1000UL);

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
    Serial << F(" expected = ") << N_PACKETS << endl;
    Serial << F("Percent intact = ") << (N_PACKETS * 100UL) / packetsRx << endl;
    // reset for next loop
    packetsRx = 0;
  }
}

// use a red color scheme, as any flicker (blue or green) is readily notable.
void doLEDs() {
  // let's push to a reasonable update rate
  const unsigned long fps = 50;
  // that means we want to push the frame on this interval
  static Metro updateInterval(1000UL / fps);

  // if it's not time to send, return.
  if ( !updateInterval.check() ) return;

  // otherwise, show how long we've been operating in binary on the first 32 LEDs, in red.
  unsigned long now = millis();
  for( int b=0; b<32; b++ ) {
    leds[b] = bitRead(now, b) ? CRGB::Red : CRGB::Black;
  }
  
  // send it.
  FastLED.show();
  
  // ready for next push.
  updateInterval.reset();

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
