/*
Warning: PLEASE DO NOT REMOVE THIS COMMENT WHEN REDISTRIBUTING!
This is the first official release of the Phi-super-font library version 20110501

Programmed by Dr. John Liu
Revision: 03/22/2011
Free software for educational and personal uses.
No warrantee!
Commercial use without authorization is prohibited.
Find details of the Phi-2 shield, and Phi-super-font or contact Dr. Liu at
http://liudr.wordpress.com/phi-2-shield/
http://liudr.wordpress.com/phi_super_font/
All rights reserved.
Update:
03/22/2012: Official release again
12/18/2011: Modified to run on both arduino IDE 1.0 and pre-1.0 versions
05/01/2011 First official release of the library
*/
#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif
#include <LiquidCrystal.h>
void invert_super_font(boolean inv);
void lcd_clear();
void super_msg_lcd(char* msg_line, byte loc_x, byte loc_y);
void render_super_number(int number, byte loc_x, byte loc_y);
void render_super_msg(char msg[], byte loc_x, byte loc_y);
void render_super_char(char ch, byte loc_x, byte loc_y);
void render_super_char_0(char ch);
void render_super_char_1(char ch);
void render_super_char_2(char ch);
void render_super_char_3(char ch);
// MGD: moved to LCD_I2C
void init_super_font(LiquidCrystal_I2C *l);
//void init_super_font(LiquidCrystal *l);
void display_img(byte msg[]);


