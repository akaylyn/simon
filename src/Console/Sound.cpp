#include "Sound.h"

bool Sound::begin() {
  Serial << "Sound: startup." << endl;

  // setup comms to Music
  Music.begin(MUSIC_COMMS_RATE);

  // setup EasyTransfer
  Serial << "EasyTransfer: pre-startup." << endl;
  easyTransfer.begin(details(message), &Music);
  Serial << "EasyTransfer: startup." << endl;

  // set the default volume
  setVolume(MUSIC_DEFAULT_VOL);
  stop();

  // Fx board
  Serial << F("Fx: setup.") << endl;
  
  // setup pins
  for(int i=0; i<N_TRIGGER; i++) {
    digitalWrite(pin[i], HIGH);
    pinMode(pin[i],OUTPUT);
  }
  
  // reset the board
  pinMode(FX_RESET, OUTPUT);
  digitalWrite(FX_RESET, LOW);
  delay(10);
  digitalWrite(FX_RESET, HIGH);

  Serial << "Sound: startup complete." << endl; 

  return ( Music ); // if there was an error with Serial connection to Music, return false.
}

void Sound::playWin(int playCount) {
  message.type = TYPE_WIN;
  message.playCount = playCount;
  sendData();
}

void Sound::playLose() {
  message.type = TYPE_LOSE;
  sendData();
}

void Sound::playBaff() {
  message.type = TYPE_BAFF;
  sendData();
}

void Sound::playRock(int playCount) {
  message.type = TYPE_ROCK;
  message.playCount = playCount;
  sendData();
}

void Sound::playTone(byte colorIndex, boolean correctTone) {
  if ( correctTone ) {
    switch ( colorIndex ) {
      case I_RED: fxOn(TR_RED); break;
      case I_GRN: fxOn(TR_GRN); break;
      case I_BLU: fxOn(TR_BLU); break;
      case I_YEL: fxOn(TR_YEL); break;
    }
  } else {
    fxOn(TR_WRONG);
  }
}

void Sound::fxOn(Trigger t) {
  digitalWrite(pin[t], LOW);
//  Serial << "Fx: " << t << " pin: " << pin[t] << endl;
}

void Sound::fxOff(Trigger t) {
  digitalWrite(pin[t], HIGH);
}

void Sound::fxAllOff() {
  // all off
  for(int i=0; i<N_TRIGGER; i++) {
    digitalWrite(pin[i], HIGH);
  }
}

void Sound::setVolume(int level) {
  char send = '0' + constrain(level, MUSIC_MIN_VOL, MUSIC_MAX_VOL);
  volume = send - '0';
  message.volume = send;
  sendData();
}

void Sound::incVolume() {
  setVolume(volume++);
}

void Sound::decVolume() {
  setVolume(volume--);
}

void Sound::stop() {
//  noTone(SPEAKER_WIRE);
  fxAllOff();
 
  message.type = TYPE_STOP;
  sendData();
}

void Sound::sendData() {
  easyTransfer.sendData();

  // reset message parameters
  // not every usage of the message structure sets all values
  message.type = TYPE_DEFAULT;
  message.playCount = PLAY_COUNT_DEFAULT;
}

Sound sound;
