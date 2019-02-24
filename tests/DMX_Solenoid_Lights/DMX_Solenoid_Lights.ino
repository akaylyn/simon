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

#define FLAME_OFFSET 1 // should read "001"
void lamp1On() { DmxSimple.write(FLAME_OFFSET+0, 255); }
void lamp1Off() { DmxSimple.write(FLAME_OFFSET+0, 0); }
void lamp2On() { DmxSimple.write(FLAME_OFFSET+1, 255); }
void lamp2Off() { DmxSimple.write(FLAME_OFFSET+1, 0); }
#define LIGHT_OFFSET 3 // should read "d003", "d007", "d011", "d014"
void red(byte level) { DmxSimple.write(LIGHT_OFFSET+0, level); }
void green(byte level) { DmxSimple.write(LIGHT_OFFSET+1, level); }
void blue(byte level) { DmxSimple.write(LIGHT_OFFSET+2, level); }
void white(byte level) { DmxSimple.write(LIGHT_OFFSET+3, level); }
void strobe(byte level) { DmxSimple.write(LIGHT_OFFSET+4, level); }
void all(byte level) { red(level); green(level); blue(level); white(level); }


void setup() {
  Serial.begin(115200);
  pinMode(3, OUTPUT);

  /* The most common pin for DMX output is pin 3, which DmxSimple
  ** uses by default. If you need to change that, do it here. */
  DmxSimple.usePin(3);

  /* DMX devices typically need to receive a complete set of channels
  ** even if you only need to adjust the first channel. You can
  ** easily change the number of channels sent here. If you don't
  ** do this, DmxSimple will set the maximum channel number to the
  ** highest channel you DmxSimple.write() to. */
  DmxSimple.maxChannel(7);

  white(255);
  strobe(0);
}


void loop() {

  // firing
  all(255);
  for(byte i=0;i<5;i++) {
    lamp1On();
    delay(75);
    lamp1Off();
    delay(100);
  }
  all(0);

  // rotate colors
  static byte color = 3;
  color++;
  if(color>=4) color=0;
  
  if(color==0) red(255);
  else if(color==1) green(255);
  else if(color==2) blue(255);
  if(color==3) white(255);

  delay(1000);  
}
