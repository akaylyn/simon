#ifndef Music_h
#define Music_h

// Serial speed.  match on the other side.
#define SERIAL_SPEED 19200 // baud

// LED pin to show we're playing
#define LED_PIN 13

// minimum volume to set [0-255].  0 is loudest
#define MIN_MP3_VOL 100

// See: https://learn.adafruit.com/adafruit-vs1053-mp3-aac-ogg-midi-wav-play-and-record-codec-tutorial/simple-audio-player-wiring
// and player_interrupts example

// VS1053 mp3:
// SPI library requirements: http://arduino.cc/en/Reference/SPI
#define MP3_CLK 13 // SPI CLK
#define MP3_MISO 12 // SPI MISO
#define MP3_MOSI 11 // SPI MOSI
#define MP3_DREQ 2 // IRQ 1; set to 3 in the example; VS1053 Data request, ideally an Interrupt pin; called DREQ in the example
#define MP3_XDCS 3 // can move; set to 8 in the example; VS1053 Data/command select pin (output); called BREAKOUT_DCS in the example
#define MP3_SDCS 4 // can move; set to 4 in the example;  Card chip select pin; called CARDCS in the example
#define MP3_CS 5 // can move; set to 10 in example; VS1053 chip select pin (output); called BREAKOUT_CS in the example
#define MP3_RST 6 // can move; set to 9 in the example; VS1053 reset pin (output); called BREAKOUT_RESET in the example


// holds the number of possible tracks to play in each gesture
// also holds the location
typedef struct
{
    const char *loc;
    int count;
} DirInfo;

#endif

