// **************************************************************
//     Filename: wavTrigger.h
// Date Created: 2/23/2014
//
//     Comments: Robertsonics WAV Trigger serial control library
//
// Programmers: Jamie Robertson, jamie@robertsonics.com
//
// **************************************************************
//
// Revision History
//
// Date      Description
// --------  -----------
//
// 02/22/14  First version created.
//           LIMITATIONS: Hard-coded for AltSoftwareSerial Library.
//           Also only supports commands TO the WAV Trigger. Will
//           fix these things.
//
// 05/10/14  Tested with UNO. Added new functions for fades, cross-
//           fades and starting multiple tracks in sample sync.


#ifndef WAVTRIGGER_H
#define WAVTRIGGER_H

#define CMD_TRACK_CONTROL	0x03
#define CMD_STOP_ALL		0x04
#define CMD_MASTER_VOLUME	0x05
#define CMD_GET_STATUS		0x07 // MGD: added
#define CMD_TRACK_VOLUME	0x08
#define CMD_TRACK_FADE		0x0A
#define CMD_RESUME_ALL_SYNC	0x0B

#define TRK_PLAY_SOLO		0x00
#define TRK_PLAY_POLY		0x01
#define TRK_PAUSE		0x02
#define TRK_RESUME		0x03
#define TRK_STOP		0x04
#define TRK_LOOP_ON		0x05
#define TRK_LOOP_OFF		0x06
#define TRK_LOAD		0x07

// MGD
//#include "../AltSoftSerial/AltSoftSerial.h"
#include "Arduino.h"
#include "Stream.h"

class wavTrigger
{
public:
	wavTrigger() {;}
	~wavTrigger() {;}
// MGD
//	void start(void);
	void start(Stream *theStream);
	void masterGain(int gain);
	void stopAllTracks(void);
	void resumeAllInSync(void);
	void trackPlaySolo(int trk);
	void trackPlayPoly(int trk);
	void trackLoad(int trk);
	void trackStop(int trk);
	void trackPause(int trk);
	void trackResume(int trk);
	void trackLoop(int trk, bool enable);
	void trackGain(int trk, int gain);
	void trackFade(int trk, int gain, int time, bool stopFlag);
	void trackCrossFade(int trkFrom, int trkTo, int gain, int time);
	// **************************************************************
	// MGD: implement GET_STATUS (Tx) and STATUS (Rx) to return playing track numbers
	// See: http://robertsonics.com/wav-trigger-online-user-guide/
	void getPlayingTracks(int playingTracks[14]);

private:
	void trackControl(int trk, int code);

//	AltSoftSerial WTSerial;
	// MGD
	Stream *WTSerial;

};

#endif
