// **************************************************************
//     Filename: wavTrigger.cpp
// Date Created: 2/23/2014
//
//     Comments: Robertsonics WAV Trigger serial control library
//
// Programmers: Jamie Robertson, jamie@robertsonics.com
//
// **************************************************************

#include "wavTrigger.h"


// **************************************************************
/* MGD
void wavTrigger::start(void) {

  WTSerial.begin(57600);
}
*/
void wavTrigger::start(Stream *theStream) {
  // calling routine responsible for setting buadrate
  WTSerial = theStream;
}

// **************************************************************
void wavTrigger::masterGain(int gain) {

byte txbuf[8];
unsigned short vol;

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_MASTER_VOLUME;
  vol = (unsigned short)gain;
  txbuf[4] = (byte)vol;
  txbuf[5] = (byte)(vol >> 8);
  txbuf[6] = 0x55;
// MGD
  //  WTSerial.write(txbuf, 7);
  WTSerial->write(txbuf, 7);
}

// **************************************************************
void wavTrigger::trackPlaySolo(int trk) {
  
  trackControl(trk, TRK_PLAY_SOLO);
}

// **************************************************************
void wavTrigger::trackPlayPoly(int trk) {
  
  trackControl(trk, TRK_PLAY_POLY);
}

// **************************************************************
void wavTrigger::trackLoad(int trk) {
  
  trackControl(trk, TRK_LOAD);
}

// **************************************************************
void wavTrigger::trackStop(int trk) {

  trackControl(trk, TRK_STOP);
}

// **************************************************************
void wavTrigger::trackPause(int trk) {

  trackControl(trk, TRK_PAUSE);
}

// **************************************************************
void wavTrigger::trackResume(int trk) {

  trackControl(trk, TRK_RESUME);
}

// **************************************************************
void wavTrigger::trackLoop(int trk, bool enable) {
 
  if (enable)
    trackControl(trk, TRK_LOOP_ON);
  else
    trackControl(trk, TRK_LOOP_OFF);
}

// **************************************************************
void wavTrigger::trackControl(int trk, int code) {
  
byte txbuf[8];

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x08;
  txbuf[3] = CMD_TRACK_CONTROL;
  txbuf[4] = (byte)code;
  txbuf[5] = (byte)trk;
  txbuf[6] = (byte)(trk >> 8);
  txbuf[7] = 0x55;
// MGD
//  WTSerial.write(txbuf, 8);
  WTSerial->write(txbuf, 8);
}

// **************************************************************
void wavTrigger::stopAllTracks(void) {

byte txbuf[5];

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_STOP_ALL;
  txbuf[4] = 0x55;
 // MGD
// WTSerial.write(txbuf, 5);
 WTSerial->write(txbuf, 5);
}

// **************************************************************
void wavTrigger::resumeAllInSync(void) {

byte txbuf[5];

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_RESUME_ALL_SYNC;
  txbuf[4] = 0x55;
// MGD
//  WTSerial.write(txbuf, 5);
  WTSerial->write(txbuf, 5);
}

// **************************************************************
void wavTrigger::trackGain(int trk, int gain) {

byte txbuf[9];
unsigned short vol;

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x09;
  txbuf[3] = CMD_TRACK_VOLUME;
  txbuf[4] = (byte)trk;
  txbuf[5] = (byte)(trk >> 8);
  vol = (unsigned short)gain;
  txbuf[6] = (byte)vol;
  txbuf[7] = (byte)(vol >> 8);
  txbuf[8] = 0x55;
  // MGD
//  WTSerial.write(txbuf, 9);
  WTSerial->write(txbuf, 9);
}

// **************************************************************
void wavTrigger::trackFade(int trk, int gain, int time, bool stopFlag) {

byte txbuf[12];
unsigned short vol;

  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (byte)trk;
  txbuf[5] = (byte)(trk >> 8);
  vol = (unsigned short)gain;
  txbuf[6] = (byte)vol;
  txbuf[7] = (byte)(vol >> 8);
  txbuf[8] = (byte)time;
  txbuf[9] = (byte)(time >> 8);
  txbuf[10] = stopFlag;
  txbuf[11] = 0x55;
// MGD
//  WTSerial.write(txbuf, 12);
  WTSerial->write(txbuf, 12);
}

// **************************************************************
void wavTrigger::trackCrossFade(int trkFrom, int trkTo, int gain, int time) {

byte txbuf[12];
unsigned short vol;

  // Start the To track with -40 dB gain
  trackGain(trkTo, -40);
  trackPlayPoly(trkTo);

  // Start a fade-in to the target volume
  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (byte)trkTo;
  txbuf[5] = (byte)(trkTo >> 8);
  vol = (unsigned short)gain;
  txbuf[6] = (byte)vol;
  txbuf[7] = (byte)(vol >> 8);
  txbuf[8] = (byte)time;
  txbuf[9] = (byte)(time >> 8);
  txbuf[10] = 0x00;
  txbuf[11] = 0x55;
// MGD
//  WTSerial.write(txbuf, 12);
  WTSerial->write(txbuf, 12);

  // Start a fade-out on the From track
  txbuf[0] = 0xf0;
  txbuf[1] = 0xaa;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (byte)trkFrom;
  txbuf[5] = (byte)(trkFrom >> 8);
  vol = (unsigned short)-40;
  txbuf[6] = (byte)vol;
  txbuf[7] = (byte)(vol >> 8);
  txbuf[8] = (byte)time;
  txbuf[9] = (byte)(time >> 8);
  txbuf[10] = 0x01;
  txbuf[11] = 0x55;
// MGD
//  WTSerial.write(txbuf, 12);
  WTSerial->write(txbuf, 12);
}


// **************************************************************
// MGD: implement GET_STATUS (Tx) and STATUS (Rx) to return playing track numbers
// See: http://robertsonics.com/wav-trigger-online-user-guide/
void wavTrigger::getPlayingTracks(int playingTracks[14]) {

	byte txbuf[5];

	txbuf[0] = 0xf0; // SOM header byte 1
	txbuf[1] = 0xaa; // SOM header byte 2
	txbuf[2] = 0x05; // message length
	txbuf[3] = CMD_GET_STATUS;
	txbuf[4] = 0x55; // EOM byte
	WTSerial->write(txbuf, 5);

	// return message is (SOM1, SOM2, LEN, MSGTYPE, # tracks *2, EOM)
	const int messageLength = 4+2*14+1;
	static char rxbuf[messageLength]; // static to preserve malloc()

	// get the message, waiting for message to complete.
	// baud rate is 57600 kps 8N1, so bitrate is 10/57600 = 0.1736 ms/byte
	// so, 33 bytes (max message size) should take 5.72 ms.
	WTSerial->setTimeout(7UL);
	int rxLen = WTSerial->readBytes(rxbuf, messageLength);

	byte msgLength = rxbuf[2];
	byte nTracks = (msgLength - 5)/2;

/*
	Serial.print("wavGPT: rxlen="); Serial.println(rxLen);
	Serial.print("wavGPT: rxbuf=");
	for( int i=0; i<rxLen; i++) {
		Serial.print(rxbuf[i],HEX);
		Serial.print(' ');
	}
	Serial.println(' ');

	Serial.print("wavGPT: nTracks=");
	Serial.println(nTracks);

	Serial.print("wavGPT: tracks="); 
*/	
	// extract playing track numbers
	for( int tr=0; tr<14; tr++ ) {
		if( tr >= nTracks ) {
			playingTracks[tr] = 0; // not playing, so zero out to indicate
		} else {
			// LSB, MSB
			// apparently, track 101 is returned as 100
			playingTracks[tr] = word(rxbuf[4+tr*2+1], rxbuf[4+tr*2]) +1;
/*
			Serial.print("Word from indexes = ");
			Serial.print(4+tr*2);
			Serial.print(' ');
			Serial.print(4+tr*2+1);
			Serial.print(' ');
*/
		}
//		Serial.println(playingTracks[tr]);
	}
}


