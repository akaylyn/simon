#ifndef SoundMessage_h
#define SoundMessage_h

// Data Transfer; Sound Types
#define TYPE_STOP 0
#define TYPE_BAFF 1
#define TYPE_WIN 2
#define TYPE_LOSE 3
#define TYPE_ROCK 4
#define TYPE_VOLUME 5

#define TYPE_DEFAULT 0
#define PLAY_COUNT_DEFAULT 1

// Used for instructing the music module on what to play.
typedef struct {
    int type;
    int playCount;
    int volume;
} SoundMessage;
#endif

