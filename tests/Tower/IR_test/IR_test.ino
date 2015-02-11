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

/* from http://forum.osmc.tv/showthread.php?tid=7142

      led-brighter            0x3AC5
          led-dim               0xBA45
          led-power             0x02FD
          
          
          led-red               0x1AE5
          led-green             0x9A65
          led-blue              0xA25D
          led-white        0x22DD
          
          
          led-redorange        0x2AD5
          led-warmwhite        0x12ED
          

          led-orange        0x0AF5
          led-pink        0x32CD
          

          led-yellow        0x38C7
          led-cyan        0xB847
          led-lightpurple    0x7887
          led-turquoise        0xF807
          

          led-greenyellow    0x18E7
          led-babyblue        0x9867
          led-purple        0x58A7
          led-bluewhite        0xD827          
          

          led-redup             0x28D7
          led-greenup           0xA857
          led-blueup            0x6897
          led-faster            0xE817
          

          led-reddown           0x08F7
          led-greendown         0x8877
          led-bluedown          0x48B7
          led-slower            0xC837
          

          led-diy1              0x30CF
          led-diy2              0xB04F
          led-diy3              0x708F
          led-auto         0xF00F
          

          led-diy4              0x10EF
          led-diy5              0x906F
          led-diy6              0x50AF
          led-flash        0xD02F
          

          led-jump3        0x20DF
          led-jump7        0xA05F
          led-fade3        0x609F
          led-fade7        0xE01F
          
*/
/* or: for a 44-key remote:
          LED_UP                0x3AC5
          LED_DOWN              0xBA45
          LED_Power_ON          0x827D
          LED_Power_OFF         0x02FD

          LED_RED               0x1AE5
          LED_GREEN             0x9A65
          LED_BLUE              0xA25D
          LED_WHITE             0x22DD

          LED_ORANGE            0x2AD5
          LED_PEE-GREEN         0xAA55 
          LED_DARK-BLUE         0x926D
          LED_MILK-WHITE        0x12ED

          LED_DARK-YELLOW       0x0AF5
          LED_CYAN              0x8A75 
          LED_LYON-BLUE         0xB24D
          LED_WHITE-PINK        0x32CD

          LED_YELLOW            0x38C7
          LED_LIGHT-BLUE        0xB847
          LED_PINK              0x7887
          LED_GREEN-WHITE       0xF807

          LED_LIGHT-YELLOW      0x18E7
          LED_SKY-BLUE          0x9867
          LED_BROWN             0x58A7
          LED_BLUE-WHITE        0xD827

          RED_UP                0x28D7
          GREEN_UP              0xA857
          BLUE_UP               0x6897
          QUICKER               0xE817

          RED_DOWN              0x08F7
          GREEN_DOWN            0x8877
          BLUE_DOWN             0x48B7
          SLOWER                0xC837

          DIY1                  0x30CF
          DIY2                  0xB04F
          DIY3                  0x708F
          AUTO                  0xF00F

          DIY4                  0x10EF
          DIY5                  0x906F
          DIY6                  0x50AF
          FLASH                 0xD02F

          JUMP3                 0x20DF
          JUMP7                 0xA05F
          FADE3                 0x609F
          FADE7                 0xE01F

*/
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


