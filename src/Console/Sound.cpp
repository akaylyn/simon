#include "Sound.h"

bool Sound::begin(){
    Serial << F("Sound: startup.") << endl;

    // setup speaker
    pinMode(SPEAKER_WIRE, OUTPUT);

    // setup comms to Music
    Music.begin(MUSIC_COMMS_RATE);

    // set the default volume
    setVolume(MUSIC_DEFAULT_VOL);
    stop();

    // setup EasyTransfer
    easyTransfer.begin(details(message), &Music);

    return( Music ); // if there was an error with Serial connection to Music, return false.
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

void Sound::playTone(byte colorIndex, unsigned long duration) {
    unsigned int freq = WRONG_TONE;

    // this function will be called after buttonChanged() asserts a change.
    switch ( colorIndex ) {
        case I_RED:
            freq = RED_TONE;
            break;
        case I_GRN:
            freq = GRN_TONE;
            break;
        case I_BLU:
            freq = BLU_TONE;
            break;
        case I_YEL:
            freq = YEL_TONE;
            break;
        case I_NONE:
            freq = WRONG_TONE;
            break;
    }

    if( duration > 0 ) {
        tone(SPEAKER_WIRE, freq, duration); // not blocking
    } else {
        tone(SPEAKER_WIRE, freq); // forever
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
    noTone(SPEAKER_WIRE);
    message.type = TYPE_STOP;
    sendData();
}

void Sound::sendData() {
    Serial << F("Music: sending ") << endl;
    easyTransfer.sendData();

    // reset message parameters
    // not every usage of the message structure sets all values
    message.type = TYPE_DEFAULT;
    message.playCount = PLAY_COUNT_DEFAULT;
}

// unit test for sound module
void testSound() {
/*
    Serial << F("Music: tone test...") << endl;

    musicTone(I_RED);
    delay(500);

    musicTone(I_GRN);
    delay(500);

    musicTone(I_BLU);
    delay(500);

    musicTone(I_YEL);
    delay(500);

    // send STOP
    musicStop();

    // up volume
    musicVolumeSet(MUSIC_MAX_VOL);

    // start a unit test.
    musicSend('w');

    Serial << F("from Music: ");
    while( volumeSetting > 0 ) {
        while( Music.available() > 0) {
            Serial.print( char(Music.read()) );
        }
        delay(1000);
        musicVolumeDown();
    }
    Serial << endl;

    musicStop();

    Serial << F("Music: unit test stopped playback.") << endl;
    */
}

