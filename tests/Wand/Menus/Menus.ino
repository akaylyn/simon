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

  tree.addUsrNav(buttons, 4);
  
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

int buttons() {
  // need to define UP, DOWN, ESCAPE, CONFIRM
  int x=analogRead(J_X);
  int y=analogRead(J_Y);
  
  if( x>=1000 && y>=1000 ) return( MW_BTC ); // right/confirm
  if( x<=10 && y>=1000 ) return( MW_BTD ); // down
  if( x<=10 && y<=10 ) return( MW_BTE ); // left/escape
  if( x>=1000 && y<=10 ) return( MW_BTU ); // up  
  
  return( MW_BTNULL );
} 


