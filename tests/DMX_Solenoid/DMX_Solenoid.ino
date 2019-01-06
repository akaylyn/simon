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
  DmxSimple.maxChannel(4+5);

  white(255);
  strobe(0);
}

#define FLAME_OFFSET 1
void flameOn() { DmxSimple.write(FLAME_OFFSET+0, 255); }
void flameOff() { DmxSimple.write(FLAME_OFFSET+0, 0); }
#define LIGHT_OFFSET 5
void red(byte level) { DmxSimple.write(LIGHT_OFFSET+0, level); }
void green(byte level) { DmxSimple.write(LIGHT_OFFSET+1, level); }
void blue(byte level) { DmxSimple.write(LIGHT_OFFSET+2, level); }
void white(byte level) { DmxSimple.write(LIGHT_OFFSET+3, level); }
void strobe(byte level) { DmxSimple.write(LIGHT_OFFSET+4, level); }
void all(byte level) { red(level); green(level); blue(level); white(level); }

void loop() {
  
  all(255);  
  strobe(255);
  flameOn();
  delay(100);
  flameOff();
  all(0);
  strobe(0);

  white(64*sin(millis() % 10000UL));
  red(255*sin(millis() % 10000UL));
  delay(900+1000);
    
}
