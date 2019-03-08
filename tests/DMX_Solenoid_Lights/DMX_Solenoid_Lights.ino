/* Welcome to DmxSimple. This library allows you to control DMX stage and
** architectural lighting and visual effects easily from Arduino. DmxSimple
** is compatible with the Tinker.it! DMX shield and all known DIY Arduino
** DMX control circuits.
**
** DmxSimple is available from: http://code.google.com/p/tinkerit/
** Help and support: http://groups.google.com/group/dmxsimple       */

/* To use DmxSimple, you will need the following line. Arduino will
** auto-insert it if you select Sketch > Import Library > DmxSimple. */

#include <DmxSimple.h>
#include <Metro.h>
#include <Streaming.h>

#define LED 9

#define FLAME_CHANS 2 // should read "001"
void lamp1On() { DmxSimple.write(1, 255); }
void lamp1Off() { DmxSimple.write(1, 0); }
void lamp2On() { DmxSimple.write(2, 255); }
void lamp2Off() { DmxSimple.write(2, 0); }
#define LAMP_CHANS 5 // should read "d003", "d008", "d013", "d018"
// Green:d003, Red:d008, Blue: d013, Yellow: d018
#define LAMP_N 4
int lampIndex(byte lamp, byte channel) {
  int ret = (int)FLAME_CHANS+(int)LAMP_CHANS*(int)lamp+(int)channel;
//  Serial << ret << endl;
  return(ret);
}
void master(byte lamp, byte level) { DmxSimple.write(lampIndex(lamp,1), level); }
void red(byte lamp, byte level) { DmxSimple.write(lampIndex(lamp,2), level); }
void green(byte lamp, byte level) { DmxSimple.write(lampIndex(lamp,3), level); }
void blue(byte lamp, byte level) { DmxSimple.write(lampIndex(lamp,4), level); }
void white(byte lamp, byte level) { DmxSimple.write(lampIndex(lamp,5), level); }
void all(byte lamp, byte level) { red(lamp, level); green(lamp, level); blue(lamp, level); white(lamp, level); }

#define FOR_ALL_LAMPS for(byte l=0;l<LAMP_N;l++)

void setup() {
  Serial.begin(115200);
  
  delay(1000);
  
  pinMode(3, OUTPUT);
  pinMode(LED, OUTPUT);

  /* The most common pin for DMX output is pin 3, which DmxSimple
  ** uses by default. If you need to change that, do it here. */
  DmxSimple.usePin(3);

  /* DMX devices typically need to receive a complete set of channels
  ** even if you only need to adjust the first channel. You can
  ** easily change the number of channels sent here. If you don't
  ** do this, DmxSimple will set the maximum channel number to the
  ** highest channel you DmxSimple.write() to. */
  DmxSimple.maxChannel(FLAME_CHANS+LAMP_N*LAMP_CHANS);

  // max level
  lamp1Off();
  lamp2Off();
  // For no reason that's obvious to me, we need to initialize to one past the number of lamps we have.  worrisome.
  for(byte i=0;i<=LAMP_N;i++) { master(i,255); red(i,0); green(i,0); blue(i,0); white(i,16); }

}


void loop() {

  // firing
  FOR_ALL_LAMPS all(l,255);
  for(byte i=0;i<5;i++) {
    lamp1On();
    delay(75);
    lamp1Off();
    delay(100);
  }
  FOR_ALL_LAMPS all(l,0);

  // rotate colors
  static byte color = 3;
  color++;
  if(color>=4) color=0;

  FOR_ALL_LAMPS all(l,0);
  if(color==0) FOR_ALL_LAMPS red(l,255); //for(byte i=0;i<=LAMP_N;i++) red(i,255);
  else if(color==1) FOR_ALL_LAMPS green(l,255); //for(byte i=0;i<=LAMP_N;i++) green(i,255);
  else if(color==2) FOR_ALL_LAMPS blue(l,255); //for(byte i=0;i<=LAMP_N;i++) blue(i,255);
  if(color==3) FOR_ALL_LAMPS white(l,255); //for(byte i=0;i<=LAMP_N;i++) white(i,255);

  static boolean ledState = false;
  ledState = !ledState;
  digitalWrite(LED, ledState);

  if(ledState) lamp2On();
  else lamp2Off();
  
  delay(1000);  
}
