//The full code is in library example file Quick_tour.ino
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <buttons.h>
#include <MENWIZ.h>
#include <EEPROM.h>
#include <Streaming.h>

#define B_RED 4
#define B_GRN 5
#define B_BLU 3
#define B_YEL 6
#define B_STK 7
#define J_X A0
#define J_Y A1
#define GROUND 2 // Wire to GND
// add wire from +3.3V to "5V" on shield.

menwiz tree;
// create lcd obj using LiquidCrystal lib
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

int  list,sp=110;

void setup(){
  _menu *r,*s1,*s2;

  Serial.begin(115200);    
  tree.begin(&lcd,20,4); //declare lcd object and screen size to menwiz lib

  r=tree.addMenu(MW_ROOT,NULL,F("Root"));
  s1=tree.addMenu(MW_SUBMENU,r, F("Node1"));
    s2=tree.addMenu(MW_VAR,s1, F("Node3"));
      s2->addVar(MW_LIST,&list);
      s2->addItem(MW_LIST, F("Option1"));
      s2->addItem(MW_LIST, F("Option2"));
      s2->addItem(MW_LIST, F("Option3"));
    s2=tree.addMenu(MW_VAR,s1, F("Node4"));
    s2->addVar(MW_AUTO_INT,&sp,0,120,10);  
  s1=tree.addMenu(MW_VAR,r, F("Node2"));
    s1->addVar(MW_ACTION,myfunc);
  //      tree.navButtons(UP_BOTTON_PIN,DOWN_BOTTON_PIN,LEFT_BOTTON_PIN,RIGHT_BOTTON_PIN,ESCAPE_BOTTON_PIN,CONFIRM_BOTTON_PIN);

  tree.addUsrNav(menuButtons, 4);
  
  pinMode(GROUND, OUTPUT);
  digitalWrite(GROUND, LOW); // wire to GN
  
  // buttons
  pinMode(B_RED, INPUT); 
  pinMode(B_GRN, INPUT); 
  pinMode(B_BLU, INPUT); 
  pinMode(B_YEL, INPUT); 
  pinMode(B_STK, INPUT); 
  
  // analog
  pinMode(J_X, INPUT);
  pinMode(J_Y, INPUT);

}

void loop(){
  tree.draw(); 
}

void myfunc(){
  Serial.println("ACTION FIRED");
}

int menuButtons() {
  // need to define UP, DOWN, ESCAPE, CONFIRM
  int x=analogRead(J_X);
  int y=analogRead(J_Y);
  
  static int lastState, currentState;
  // offset from extremes to score as "yes"
  const int offSet = 100;
  
  // do want to allow for change-when-held, but only after a delay
  static unsigned long lastChange = millis();
  const unsigned long pressAgainInterval = 200UL;
  
  if( x>=(1023-offSet) && y>=(1023-offSet) ) currentState = MW_BTC; // right/confirm: +x, +y
  else if( x<=(0+offSet) && y>=(1023-offSet) ) currentState = MW_BTD; // down: -x, +y
  else if( x<=(0+offSet) && y<=(0+offSet) ) currentState = MW_BTE; // left/escape: -x, -y
  else if( x>=(1023-offSet) && y<=(0+offSet) ) currentState = MW_BTU; // up: +x, -y
  else currentState = MW_BTNULL; // nada
  
  if( currentState == lastState && (millis() - lastChange) <= pressAgainInterval) {
    // nothing new
    return( MW_BTNULL );
  } else {
    lastState = currentState;
    lastChange = millis();
    return( currentState );
  }
} 


