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
#include <stdio.h>
#include <avr/pgmspace.h>
// MGD moved to LCD_I2C
#include <LiquidCrystal_I2C.h>
//#include <LiquidCrystal.h>
#include "phi_super_font.h"
#include "characters.h"

#ifdef sparsh
const char psf_lcd_ch0[] PROGMEM ={64,64,64,64,64,64,64,64,0};// 0
const char psf_lcd_ch1[] PROGMEM ={64,64,64,64,14,14,14,14,0};//1
const char psf_lcd_ch2[] PROGMEM ={14,14,14,14,64,64,64,64,0};//2
const char psf_lcd_ch3[] PROGMEM ={14,14,14,14,14,14,14,14,0};//3
#else
#ifdef sparsh_circle
const char psf_lcd_ch0[] PROGMEM ={64,64,64,64,64,64,64,64,0};// 0
const char psf_lcd_ch1[] PROGMEM ={64,64,64,64,14,31,31,14,0};//1
const char psf_lcd_ch2[] PROGMEM ={14,31,31,14,64,64,64,64,0};//2
const char psf_lcd_ch3[] PROGMEM ={14,31,31,14,14,31,31,14,0};//3
#else
const char psf_lcd_ch0[] PROGMEM ={64,64,64,64,64,64,64,64,0};// 0
const char psf_lcd_ch1[] PROGMEM ={64,64,64,64,31,31,31,31,0};//1
const char psf_lcd_ch2[] PROGMEM ={31,31,31,31,64,64,64,64,0};//2
const char psf_lcd_ch3[] PROGMEM ={31,31,31,31,31,31,31,31,0};//3
#endif
#endif

#ifndef character_spacing
#define character_spacing 1
#endif

const char* const ch_item[] PROGMEM = {psf_lcd_ch0, psf_lcd_ch1, psf_lcd_ch2, psf_lcd_ch3};

// MGD moved to LCD_I2C
LiquidCrystal_I2C *output_lcd;
// LiquidCrystal *output_lcd;

boolean inverted_super_font=false;

// MGD moved to LCD_I2C
void init_super_font(LiquidCrystal_I2C *l)
//void init_super_font(LiquidCrystal *l)
{
  byte ch_buffer[10]; // This buffer is required for custom characters on the LCD.
  output_lcd=l;
  for (int i=0;i<4;i++)
  {
    strcpy_P((char*)ch_buffer,(char*)pgm_read_word(&(ch_item[i])));
    output_lcd->createChar(i, ch_buffer);
  }
}

void render_super_char_3(char ch)
{
  if ((ch>=' ')&&(ch<='~'))
  {
    for (byte i=0; i<5; i++)
    {
      byte x=(font[(ch-' ')][i])>>6;
      if (!inverted_super_font) output_lcd->write((byte)((x>>1)+((x&1)<<1)));
      else output_lcd->write((byte)(3-((x>>1)+((x&1)<<1))));
    }
  }
}

void render_super_char_2(char ch)
{
  if ((ch>=' ')&&(ch<='~'))
  {
    for (byte i=0; i<5; i++)
    {
      byte x=((font[(ch-' ')][i])>>4)&3;
      if (!inverted_super_font) output_lcd->write((byte)((x>>1)+((x&1)<<1)));
      else output_lcd->write((byte)(3-((x>>1)+((x&1)<<1))));
    }
  }
}

void render_super_char_1(char ch)
{
  if ((ch>=' ')&&(ch<='~'))
  {
    for (byte i=0; i<5; i++)
    {
      byte x=((font[(ch-' ')][i])>>2)&3;
      if (!inverted_super_font) output_lcd->write((byte)((x>>1)+((x&1)<<1)));
      else output_lcd->write((byte)(3-((x>>1)+((x&1)<<1))));
    }
  }
}

void render_super_char_0(char ch)
{
  if ((ch>=' ')&&(ch<='~'))
  {
    for (byte i=0; i<5; i++)
    {
      byte x=(font[(ch-' ')][i])&3;
      if (!inverted_super_font) output_lcd->write((byte)((x>>1)+((x&1)<<1)));
      else output_lcd->write((byte)(3-((x>>1)+((x&1)<<1))));
    }
  }
}

void render_super_char(char ch, byte loc_x, byte loc_y)
{
  output_lcd->setCursor(loc_x,loc_y);
  render_super_char_0(ch);
  output_lcd->setCursor(loc_x,loc_y+1);
  render_super_char_1(ch);
  output_lcd->setCursor(loc_x,loc_y+2);
  render_super_char_2(ch);
  output_lcd->setCursor(loc_x,loc_y+3);
  render_super_char_3(ch);
}

void render_super_msg(char msg[], byte loc_x, byte loc_y)
{
  byte i=0;
  while(msg[i])
  {
    render_super_char(msg[i],loc_x+i*(5+character_spacing), loc_y);
    i++;
  }
}

void render_super_number(int number, byte loc_x, byte loc_y)
{
  char msg[11];
  sprintf(msg,"%d",number);
  render_super_msg(msg, loc_x, loc_y);
}

//Display strings stored in PROGMEM. Provide the string name stored in PROGMEM to be displayed on the LCD's current cursor position.
void super_msg_lcd(char* msg_line, byte loc_x, byte loc_y)
{
  char msg_buffer[17];
  strcpy_P(msg_buffer,msg_line);
  msg_buffer[4]=0; 
  render_super_msg(msg_buffer, loc_x, loc_y);
}

void lcd_clear()
{
  if (!inverted_super_font) output_lcd->clear();
  else
  {
    output_lcd->setCursor(0,0);
    for (byte i=0;i<80;i++)
    {
      output_lcd->write(255);
    }
  }
}

void invert_super_font(boolean inv)
{
  inverted_super_font=inv;
}

void display_img(byte msg[])
{
  output_lcd->setCursor(0,0);
  for (byte i=0;i<80;i++)
  {
    output_lcd->write(msg[i]);
  }
}
