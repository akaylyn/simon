#include "Fanfare.h"

void wait(unsigned long time) {
  Metro delayNow(time);
  delayNow.reset(); // a good idea

  while (!delayNow.check()) {
    network.update();
  }
}

#define FANFARE_ENABLED true

void playerFanfare(byte level) {
  Serial << "***Inside alan playerFanFare: level: " << level << "\n";

  if (!FANFARE_ENABLED) {
    Serial.println("Fanfare disabled");
    return;
  }

  Serial << F("Gameplay: Player fanfare, level: ") << level << endl;

  //Metro fanfareDuration(FANFARE_DURATION_PER_CORRECT * currentLength);

  // make sweet fire/light/music.
  sound.setLeveling(0, 1); // important that you do this, or the sound will be garbage
  int winTrack = sound.playWins();

  color towerOrder[N_COLORS] = {I_RED, I_BLU, I_YEL, I_GRN}; // go clockwise around the towers

  // what bands map to what output?
  const int fireBand = 1;
  const int redBand = 2;
  const int greenBand = 4;
  const int blueBand = 6;

  mic.setThreshold(fireBand, 2);
  mic.setThreshold(redBand, 2);
  mic.setThreshold(greenBand, 2);
  mic.setThreshold(blueBand, 2);

  Metro winTime(30000);  // TODO: How do we know the length in ms of the track?

  while (!winTime.check()) {
    mic.update();

    byte nActiveChannels = 0;
    for (byte i = 0; i < NUM_FREQUENCY_BANDS; i++) {
      // sum up the number of active channels
      if ( mic.getBeat(i) ) nActiveChannels++;
    }
    boolean fireBeat = mic.getBeat(fireBand);

    static boolean midBeat = false;

    // fire.  check for a beat (and not already in a beat)
    static byte fireTower = 0;
    if (fireBeat && !midBeat) {
      fireTower++;
      if (fireTower > N_COLORS) fireTower = 0;

      // select fire from 50-500 ms based on the volume level
      byte fireLevel = map(mic.getAvg(fireBand), 0, 1024, 5, 50);
      // select flame effect from number of active channels.
      flameEffect effect = (flameEffect)map(nActiveChannels, 1, NUM_FREQUENCY_BANDS, 0, N_flameEffects);

      Serial << F("DragonFireBallZ: ") << fireLevel*10 << F("ms, avg: ") << (mic.getAvg(fireBand)) << endl;
      fire.setFire(towerOrder[fireTower], fireLevel, effect);
      
      // and we heard a beat
      midBeat = true;
    } else if (!fireBeat) {
      midBeat = false;
    }

    // lights.
    static byte lightTower = 0;
    if ( nActiveChannels == NUM_FREQUENCY_BANDS ) {
      // huge sound.  move on to the next tower.
      lightTower++;
      if (lightTower > N_COLORS) lightTower = 0;
    }

    byte red = map(mic.getAvg(redBand), 0, 1024, 0, 255);
    byte green = map(mic.getAvg(greenBand), 0, 1024, 0, 255);
    byte blue = map(mic.getAvg(blueBand), 0, 1024, 0, 255);

    light.setLight(towerOrder[lightTower], red, green, blue);

    wait(1);
  }

  light.clear();
  fire.clear();

  // ramp down the volume to exit the music playing cleanly.
  sound.fadeTrack(winTrack);

  Serial << F("Gameplay: Player fanfare ended") << endl;
}

// Get the fanfare level to play based on # correct.  Returns 0-3
byte getLevel(int correct) {
  Serial << "Get Level: correct: " << correct << "\n";
  if (correct < FANFARE_LEVEL1) {
    return FANFARE_NONE;
  } else if (correct <= FANFARE_LEVEL2) {
    return FANFARE_1;
  } else if (correct <= FANFARE_LEVEL3) {
    return FANFARE_2;
  } else {
    return FANFARE_3;
  }
}

