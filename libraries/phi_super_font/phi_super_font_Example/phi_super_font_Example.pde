/*
Warning: PLEASE DO NOT REMOVE THIS COMMENT WHEN REDISTRIBUTING!
This is an example showcasing the first official release of the Phi-super-font library version 20110501
Please obtain manual of this library from:
http://liudr.wordpress.com/phi_super_font/

Programmed by Dr. John Liu
Revision: 05/01/2011
Free software for educational and personal uses.
No warrantee!
Commercial use without authorization is prohibited.
Find details of the Phi-1 shield, Phi-2 shield, and Phi-super-font or contact Dr. Liu at
http://liudr.wordpress.com/phi-1-shield/
http://liudr.wordpress.com/phi-2-shield/

All rights reserved.
*/

#define phi_2_shield // Define this if you are using the phi 2 shield
//#define phi_1_shield // Define this if you are using the phi 1 shield
#define sparsh_circle

#include <LiquidCrystal.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <phi_super_font.h>

#include "defs.h"

LiquidCrystal lcd(LCD_RS,LCD_EN,LCD_D4,LCD_D5,LCD_D6,LCD_D7); // Create the lcd object

byte img1[]={
0,0,0,1,2,2,2,1,0,0,0,1,2,2,2,1,0,0,0,0,
0,0,2,1,0,2,0,1,2,0,2,1,0,0,0,1,2,0,0,0,
0,0,3,0,1,3,1,0,3,1,3,0,1,1,1,0,3,0,0,0,
0,0,0,0,2,2,2,0,0,0,0,0,2,2,2,0,0,0,0,0
};// line 0, 2, 1, 3 order.

boolean inverted=false;

void setup()
{
  lcd.begin(20, 4);
  init_super_font(&lcd);
  show_credit();
  show_arduino();
}

void loop()
{
  demo_20X4();
}

void demo_20X4() //Asks you for password for a given number of trials
{
  char msg[4];

  lcd_clear();
  render_super_msg("Sup", 0,0);
  delay(2000);
  
  render_super_msg("Fnt", 0,0);
  delay(2000);
  
  inverted=!inverted;
  invert_super_font(inverted);
  lcd_clear();
  render_super_msg("Sup", 0,0);
  delay(2000);
  
  render_super_msg("Fnt", 0,0);
  delay(2000);
  

  inverted=!inverted;
  invert_super_font(inverted);

  char msg1[]="HI";
  byte x=0;
  byte j=0;
  for (byte i=0;i<9;i++)
  {
    lcd_clear();
    render_super_msg(msg1,i,0);
    delay(1000);
  }

// Show messages inside PROGMEM
  lcd_clear();
  for (byte i=0;i<7;i++)
  {
    super_msg_lcd(dow_00+i*4, 0,0);
    lcd.setCursor(17,0);
    msg_lcd(dow_00+i*4);
    delay(1000);
  }
  
  for (byte i=0;i<12;i++)
  {
    super_msg_lcd(month_00+i*4,0,0);
    lcd.setCursor(17,0);
    msg_lcd(month_00+i*4);
    delay(1000);
  }

// Display numbers
  lcd_clear();
  for (byte i=0;i<20;i++)
  {
    lcd.setCursor(0,0);
    render_super_number(i,0,0);
    delay(400-i*6);
  }
  
  lcd_clear();
  render_super_msg("Sup",0,0);
  delay(2000);
  render_super_msg("Chr",0,0);
  delay(2000);

// Show all characters  
  lcd_clear();
  for (byte i=' ';i<='~';i++)
  {
    if (!((i-' ')%4)) lcd_clear();
    render_super_char(i,(i-' ')%4*5, 0);
    lcd.setCursor(19,0);
    lcd.write(i);
    delay(500);
  }
  delay(1000);
  
  lcd_clear();
  render_super_msg("Dr.",0,0);
  delay(2000);
  render_super_msg("Liu",0,0);
  delay(2000);

// Invert display polarity for the next loop.
  inverted=!inverted;
  invert_super_font(inverted);
}

//Utility functions
void msg_lcd(char* msg_line)
{
  char msg_buffer[17];
  strcpy_P(msg_buffer,msg_line); 
  lcd.print(msg_buffer);
}

void show_credit()
{
  // Display credits
  lcd.clear();
  msg_lcd(msg_00);
  lcd.setCursor(0,1);
  msg_lcd(msg_01);
  delay(2000);
  lcd.clear();
  msg_lcd(msg_02);
  lcd.setCursor(0,1);
  msg_lcd(msg_03);
  delay(2000);
  lcd.clear();
}

void show_arduino()
{
  display_img(img1);
  delay(5000);
}
