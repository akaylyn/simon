// need to strike RobotIRRemote directory in Arduino IDE libraries folder.  Name collision!
#include <IRremote.h>
#include <Streaming.h>

#define LED_W 9 // the PWM pin which drives the white LED (not connected)

IRsend irsend;

#define K24_OFF          0xF740BF
#define K44_OFF          0xFF02FD
#define K24_ON           0xF7C03F
#define K44_ON           0xFF827D

#define K24_RED          0xF720DF
#define K44_RED          0xFF1AE5
#define K44_RED_UP       0xFF28D7
#define K44_RED_DOWN     0xFF08F7

#define K24_GRN          0xF7A05F
#define K44_GRN          0xFF9A65
#define K44_GRN_UP       0xFFA857
#define K44_GRN_DOWN     0xFF8877

#define K24_BLU          0xF7609F
#define K44_BLU          0xFFA25D
#define K44_BLU_UP       0xFF6897
#define K44_BLU_DOWN     0xFF48B7

#define K24_YEL          0xF728D7
#define K44_YEL          0xFF38C7

#define K24_WHT          0xF7E01F
#define K44_WHT          0xFF22DD

#define K24_DOWN         0xF7807F
#define K44_DOWN         0xFFBA45
#define K24_UP           0xF700FF
#define K44_UP           0xFF3AC5

#define K44_QUICKER      0xFFE817
#define K44_SLOWER       0xFFC837

#define K24_FLASH        0xF7D02F
#define K44_FLASH        0xFFD02F

#define K24_STROBE       0xF7F00F

#define K24_FADE         0xF7C837
#define K44_FADE3        0xFF609F
#define K44_FADE7        0xFFE01F
          
#define K24_SMOOTH       0xF7E817

#define CODE_REPEAT      0xFFFFFF

#define N_BRIGHT_STEPS   8

void setup()
{
  Serial.begin(115200);
  Serial << "Startup." << endl;
}

void loop() {
  
  Serial << "Test 24-key functions." << endl;
  send(K24_ON); Serial << "24 on." << endl;
  
  send(K24_RED); Serial << "red toggled." << endl;
  upDown(K24_UP, K24_DOWN, N_BRIGHT_STEPS, 200);
  send(K24_RED); Serial << "red toggled." << endl;

  send(K24_GRN); Serial << "grn toggled." << endl;
  upDown(K24_UP, K24_DOWN, N_BRIGHT_STEPS, 200);
  send(K24_GRN); Serial << "grn toggled." << endl;

  send(K24_BLU); Serial << "blu toggled." << endl;
  upDown(K24_UP, K24_DOWN, N_BRIGHT_STEPS, 200);
  send(K24_BLU); Serial << "blu toggled." << endl;

  send(K24_YEL); Serial << "yel toggled." << endl;
  upDown(K24_UP, K24_DOWN, N_BRIGHT_STEPS, 200);
  send(K24_YEL); Serial << "yel toggled." << endl;
  
  send(K24_WHT); Serial << "wht toggled." << endl;
  upDown(K24_UP, K24_DOWN, N_BRIGHT_STEPS, 200);
  send(K24_WHT); Serial << "wht toggled." << endl;
  
  send(K24_FADE); Serial << "fade toggled." << endl;
  sendMultiple(K24_UP, N_BRIGHT_STEPS, 5);
  delay(5000);
  sendMultiple(K24_DOWN, N_BRIGHT_STEPS, 5);
  send(K24_FADE); Serial << "fade toggled." << endl;
  
  Serial << "End 24-key tests." << endl;
  send(K24_OFF); Serial << "24 off." << endl;
  
  while(1);
}

void send(unsigned long data) {
  // simple wrapper
  irsend.sendNEC(data, 32);
}

void sendMultiple(unsigned long data, int steps, unsigned long dtime) {
  for( int i=0; i<steps; i++ ) {
    send(data); Serial << i << " ";
    delay(dtime);
  }
  Serial << endl;
}

void upDown(unsigned long up, unsigned long down, int steps, unsigned long dtime) {
  Serial << "up: ";
  sendMultiple(up, steps, dtime);
  Serial << "down: ";
  sendMultiple(down, steps, dtime);
}

/* from http://forum.osmc.tv/showthread.php?tid=7142
44-key:

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


// see http://www.vishay.com/docs/80071/dataform.pdf
// see https://github.com/RC-Navy/DigisparkArduinoIntegration/blob/master/libraries/DigisparkIRLib/examples/DigiIrRgbCtrl/DigiIrRgbCtrl.ino#L45
// 24-key controller:



/* http://forum.osmc.tv/showthread.php?tid=7142
or: for a 44-key remote:
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
