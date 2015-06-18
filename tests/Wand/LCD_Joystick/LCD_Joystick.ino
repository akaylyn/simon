#include <Streaming.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

#define B_RED 4
#define B_GRN 5
#define B_BLU 3
#define B_YEL 6
#define B_STK 7
#define J_X A0
#define J_Y A1
#define GROUND 2 // Wire to GND
// add wire from +3.3V to "5V" on shield.

void setup() {

  // need to set this up before Serial begin?
  lcd.begin(20,4);               // initialize the lcd 
  lcd.backlight();
  
  Serial.begin(115200);

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

void loop() {
  // put your main code here, to run repeatedly:
  Serial << "\tR:" << digitalRead(B_RED); 
  Serial << "\tG:" << digitalRead(B_GRN); 
  Serial << "\tB:" << digitalRead(B_BLU); 
  Serial << "\tY:" << digitalRead(B_YEL); 
  Serial << "\tS:" << digitalRead(B_STK); 
  Serial << "\tX:" << analogRead(J_X); 
  Serial << "\tY:" << analogRead(J_Y); 
  Serial << endl;
  delay(200);
  
  lcd.home();
  lcd.setCursor(0, 0);        
  lcd.print("R:");  
  lcd.print(digitalRead(B_RED));
  lcd.setCursor(5, 0);
  lcd.print("G:");  
  lcd.print(digitalRead(B_GRN));
  lcd.setCursor(10, 0);
  lcd.print("B:");  
  lcd.print(digitalRead(B_BLU));
  lcd.setCursor(15, 0);
  lcd.print("Y:");  
  lcd.print(digitalRead(B_YEL));
  
  lcd.setCursor(0, 1 );        // go to the next line
  lcd.print("X:");  
  lcd.print(analogRead(J_X));
  lcd.print("   ");
  lcd.setCursor(10, 1);
  lcd.print("Y:");  
  lcd.print(analogRead(J_Y));
  lcd.print("   ");
  delay(200);
}
