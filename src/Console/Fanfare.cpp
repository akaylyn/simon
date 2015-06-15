#include "Fanfare.h"


#define FANFARE_ENABLED true

int bassBand = 0;
int bassBand2 = 1;
int band2 = 6;
boolean hearBeat = false;
byte active = 0;
byte tower, tower2 = I_RED;

void playerFanfare(fanfare_t level) {
  Serial << "***Inside alan playerFanFare: fanfareLevel: " << level << "\n";

  if (!FANFARE_ENABLED) {
    Serial.println("Fanfare disabled");
    return;
  }

  Serial << F("Gameplay: Player fanfare, fanfareLevel: ") << level << endl;
  //Metro fanfareDuration(FANFARE_DURATION_PER_CORRECT * currentLength);

  // make sweet fire/light/music.
  sound.setLeveling(0, 1);
  int winTrack = sound.playWins();

  light.clear();

  mic.update();   // populate avg
  mic.update();   // populate avg

  float beatThreshold = 3;
  byte fireLevel = 2;  // power curve of fire fanfareLevel mapping, higher increases average fire fanfareLevel
  byte airChance = 0;  // n in 10 chance of air effect
  int fireBeat = 450;

  // stats
  int fireballs = 0;
  int firepower = 0;

  mic.setThreshold(bassBand, beatThreshold);
  mic.setThreshold(bassBand2, beatThreshold);

  Metro winTime(30000);  // TODO: How do we know the length in ms of the track?
  winTime.reset();

  Metro beatWait(333);
  beatWait.reset();

  while (!winTime.check()) {
    Metro myWait(1UL);
    myWait.reset();
    while(!myWait.check()) network.update(); // resends, etc.

    mic.update();

    for (byte i = 0; i < NUM_FREQUENCY_BANDS; i++) {
      if ( mic.getBeat(i) ) {
        active++;
      }
    }

    //      if ((mic.getBeat(bassBand) || mic.getBeat(bassBand2)) && hearBeat == false) {
    if (beatWait.check() && (mic.getVol(bassBand) > fireBeat || mic.getVol(bassBand2) > fireBeat) && hearBeat == false) {
      tower++;
      if (tower >= N_COLORS) tower = I_RED;
      byte avgVol = max(mic.getAvg(bassBand), mic.getAvg(bassBand2));
      //        Serial << "avgVol: " << avgVol << " fscale: " << fscale(0, 1024, 0, 255, avgVol, fireLevel) << endl;
      //Serial << "band1: " << mic.getAvg(bassBand) << " band2: " << mic.getAvg(bassBand) << endl;

      byte fireLevel = floor(fscale(0, 1024, 50, 255, avgVol, fireLevel));
      flameEffect airEffect = veryRich;

      if (random(0, 10) >= (10 - airChance)) {
        byte effect = random(0, 6);
        switch (effect) {
          case 0:
            airEffect = kickStart;
            break;
          case 1:
            airEffect = kickMiddle;
            break;
          case 2:
            airEffect = kickEnd;
            break;
          case 3:
            airEffect = gatlingGun;
            break;
          case 4:
            airEffect = randomly;
            break;
          case 5:
            airEffect = veryLean;
            break;
        }
      }
      //Serial << "Fireballv3: " << (fire) << " air: " << (airEffect) << endl;
      // I really thinkg you mean "setFire(I_RED)" or some directive to a tower.
      fire.setFire((color)tower, fireLevel, airEffect);
      hearBeat = true;
      fireballs++;
      firepower += fireLevel;
      beatWait.interval(333);
      beatWait.reset();

    } else if (!(mic.getVol(bassBand) > fireBeat || mic.getVol(bassBand2) > fireBeat)) {
      //light.setAllFire(0);  // TODO: Not sure we really want to shut off flame here
      //light.show();
      hearBeat = false;
    }
    // clear packet
    light.clear();
    fire.clear();

    if (active > 0) {
      switch (active) {
        case 0:
          // light.setAllLight(0);
          light.clear();
          break;
        case 1:
        case 2:
          //light.setLight(I_RED, 255);
          light.setLight((color)tower2, 255, 0 , 0);
          break;
        case 3:
        case 4:
          //light.setLight(I_GRN, 255);
          light.setLight((color)tower2, 0, 255, 0);
          break;
        case 5:
        case 6:
          //light.setLight(I_BLU, 255);
          light.setLight((color)tower2, 0, 0, 255);
          break;
        default:
          // light.setAllLight(255);
          light.setLight((color)tower2, 255, 255, 255);
          active = 0;
          break;
      }

      //light.show(tower2);
      if (random(10) > 2) {
        tower2++;
      }
      if (tower2 >= N_COLORS) tower2 = I_RED;
    }
  }

  light.clear();
  fire.clear();

  // ramp down the volume to exit the music playing cleanly.
  sound.fadeTrack(winTrack);

  Serial << "Fireballs: " << fireballs << " power: " << firepower << endl;
  Serial << F("Gameplay: Player fanfare ended") << endl;

}

