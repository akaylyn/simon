#include <IRremote.h>

IRsend irsend;

// see http://www.vishay.com/docs/80071/dataform.pdf
// see https://github.com/RC-Navy/DigisparkArduinoIntegration/blob/master/libraries/DigisparkIRLib/examples/DigiIrRgbCtrl/DigiIrRgbCtrl.ino#L45

#define CODE_OFF           0xF740BF
#define CODE_ON            0xF7C03F
#define CODE_BRIGHT_MINUS  0xF7807F
#define CODE_BRIGHT_PLUS   0xF700FF

#define CODE_FLASH         0xF7D02F
#define CODE_STROBE        0xF7F00F
#define CODE_FADE          0xF7C837
#define CODE_SMOOTH        0xF7E817

#define CODE_RED           0xF720DF
#define CODE_GREEN         0xF7A05F
#define CODE_BLUE          0xF7609F
#define CODE_WHITE         0xF7E01F

#define CODE_ORANGE        0xF710EF
#define CODE_ORANGE_LIGTH  0xF730CF
#define CODE_BROWN         0xF708F7
#define CODE_YELLOW        0xF728D7

#define CODE_GREEN_LIGTH   0xF7906F
#define CODE_GREEN_BLUE1   0xF7B04F
#define CODE_GREEN_BLUE2   0xF78877
#define CODE_GREEN_BLUE3   0xF7A857

#define CODE_BLUE_LIGTH    0xF750AF
#define CODE_PURPLE_DARK   0xF7708F
#define CODE_PURPLE_LIGTH  0xF748B7
#define CODE_PINK          0xF76897

#define BRIGTH_STEP        10
#define CODE_REPEAT        0xFFFFFF

void setup()
{
  Serial.begin(115200);
  irsend.sendNEC(CODE_OFF, 32);
  irsend.sendNEC(CODE_ON, 32);
}

void loop() {
  irsend.sendNEC(CODE_ON, 32);
  delay(500);

  irsend.sendNEC(CODE_RED, 32);
  delay(500);
  for (int i = 0; i < 10; i++) {
    irsend.sendNEC(CODE_BRIGHT_MINUS, 32);
    delay(500);
  }
  for (int i = 0; i < 10; i++) {
    irsend.sendNEC(CODE_BRIGHT_PLUS, 32);
    delay(500);
  }
  irsend.sendNEC(CODE_RED, 32);

  irsend.sendNEC(CODE_GREEN, 32);
  delay(500);
  irsend.sendNEC(CODE_BLUE, 32);
  delay(500);
  irsend.sendNEC(CODE_WHITE, 32);
  delay(500);
  irsend.sendNEC(CODE_OFF, 32);
  delay(500);
}


