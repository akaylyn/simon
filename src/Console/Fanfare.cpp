
#include "Fanfare.h"


#define FANFARE_ENABLED true

int bassBand = 0;
int bassBand2 = 1;
int band2 = 6;
boolean hearBeat = false;
byte active = 0;
byte tower, tower2 = I_RED;

color incColor(color val) {
  switch(val) {
    case I_RED: return I_BLU;
    case I_BLU: return I_YEL;
    case I_YEL: return I_GRN;
    case I_GRN: return I_RED;
  }
  return( I_RED ); // as a safety?

  // MGD: alternately, this would work (byte cast, increment, modulo, color cast)
//  return( (color) ((byte)val+1 % N_COLORS) );

}

color randColor() {
  switch(random(0,5)) {
     case 0: return I_RED;
     case 1: return I_BLU;
     case 2: return I_YEL;
     case 3: return I_GRN;
  }
  return I_RED;
}

color oppTower(color val) {
  switch(val) {
    case I_RED: return I_YEL;
    case I_BLU: return I_GRN;
    case I_GRN: return I_BLU;
    case I_YEL: return I_RED;
  }

  return I_RED;
}

void loseFanfare() {
    int track = sound.playLose();
    unsigned long trackLength = 3000UL;
    unsigned long currTime = millis();
    unsigned long startTime = millis();

    Serial << F("Playing lose"); // harsh
/*
    fire.setFire(I_RED,5,veryLean);
    fire.setFire(I_GRN,5,veryLean);

      while((currTime - startTime) < trackLength) {
          currTime = millis();
          network.update();
          delay(1);
      }
    Serial << "Done losing" << endl;

*/
    byte fireChance = 20;  // n chance in 100 of a tower shooting a fireball

    color fireTower = I_RED;


    // A lone small fireball as consolation on n towers
    int fireballs = 0;
    byte num = random(0,4);

    switch(num) {
      case 0:
      case 1:
        fire.setFire(I_RED,5,veryRich);
        break;
      case 2:
      case 3:
        fire.setFire(I_RED,5,veryRich);
        fire.setFire(I_GRN,5,veryRich);
        break;
    }

    unsigned long lastBlink = currTime;
    byte blinkState = 0;
    while((currTime - startTime) < trackLength) {
      currTime = millis();

      if (currTime - lastBlink > 500) {
        lastBlink = currTime;
        if (blinkState == 0) {
           blinkState = 1;
           light.setLight(I_RED, 255, 0 , 0);
           light.setLight(I_GRN, 255, 0 , 0);
           light.setLight(I_BLU, 255, 0 , 0);
           light.setLight(I_YEL, 255, 0 , 0);
        } else {
          blinkState = 0;
           light.setLight(I_RED, 255, 0 , 0);
           light.setLight(I_GRN, 255, 0 , 0);
           light.setLight(I_BLU, 255, 0 , 0);
           light.setLight(I_YEL, 255, 0 , 0);
           fire.clear();
        }
      }
      network.update();
    }

    sound.fadeTrack(track);
}

void playerFanfare(fanfare_t level) {
  if (!FANFARE_ENABLED) {
    Serial.println(F("Fanfare disabled"));
    return;
  }

  Serial << F("Gameplay: Player fanfare, fanfareLevel: ") << level << F(" consolation: ") << CONSOLATION << endl;
  //Metro fanfareDuration(FANFARE_DURATION_PER_CORRECT * currentLength);

  // make sweet fire/light/music.
  sound.setLeveling(0, 1);
  int track;

  if (level == CONSOLATION) {
    loseFanfare();
    return;
  }
  track = sound.playWins();

  light.clear();
  fire.clear();
  network.update();

  listenWav.update();   // populate avg
  listenWav.update();   // populate avg

  unsigned long trackLength = 30000UL;

  switch(level) {
    case LEVEL1:
      trackLength = 12000UL;
      break;
    case LEVEL2:
      trackLength = 18000UL;
      break;
    case LEVEL3:
      trackLength = 24000UL;
      break;
    case LEVEL4:
    case MAXOUT:
      trackLength = 30000UL;
      break;
    case IDLE:
      trackLength = 18000UL;
      break;
    case CONSOLATION:
      trackLength = 3000UL;
      break;
    case NONE:
      return;
  }

  unsigned long startTime = millis() - 1;
   Metro winTime(trackLength);  // Tracks are ~30s in length
   winTime.reset();

   unsigned long beatInterval = 333;  // 180 BPM max
   byte beatChance = 80;  // chance in 100 a beat triggers a fire.  Makes the anim for a specific track different each time
   byte airChance = 0;  // n in 100- chance of air effect
   byte lightMoveChance = 50;  // n in 100 chance of the light moving on a beat
   byte minFirePerFireball = 50;  // min fire level(ms) per fireball
   byte maxFirePerFireball = 200;  // max fire level(ms) per fireball
  
  // MGD
  //   float fireBudgetFactor = 7.0;  // Divisor of track length we throw fire.  Tune this to throw less fire
  // set from fireTest
  float fireBudgetFactor = loadFireBudgetFactor();  // Divisor of track length we throw fire.  Tune this to throw less fire

   unsigned long beatEndTime = millis();  // time left for beat effect
   unsigned long beatWaitTime = millis();
   unsigned long currTime;
   int fireballs = 0;
   unsigned long firepower = 1;
   float threshold = 1.5; // initial threshold is likely to throw a fireball
   unsigned long budget = (unsigned long) ((float)trackLength / fireBudgetFactor);
   float bt = (float) trackLength / budget;
   byte active;
   //unsigned long samples = 0;
   
  Serial << F("Track Length: ") << trackLength << F(" budget: ") << budget << endl;;

   color fireTower = I_RED;
   color lightTower = I_RED;

   // Use the threshold to meet budget constraints for fire.  Ratio of current time / total Time and fire power / budget.
   while(!winTime.check()) {
     currTime = millis();
     threshold *= bt * (float)firepower / ((float) (currTime - startTime));
     threshold = constrain(threshold,1.0,5.0);
     listenWav.setThreshold(bassBand, threshold);
     listenWav.setThreshold(bassBand2, threshold);
     network.update();
     waitDuration(1UL);
     listenWav.update();
     //samples++;
     //if (samples > 100) listenWav.print();
     

     if (hearBeat && currTime > beatEndTime) {
       Serial << F("Beat over.  ") << endl;
       light.clear();
       fire.clear();
       network.update();
       hearBeat = false;
       waitDuration(10UL);
     }

    // Lights will queue changes based on activity level across all non bass bands

    for (byte i = 2; i < NUM_FREQUENCY_BANDS; i++) {
      if ( listenWav.getBeat(i) ) {
        active++;
      }
    }

    if (active > 0) {
      switch (active) {
      case 0:
        light.clear();
        break;
      case 1:
      case 2:
        light.setLight(lightTower, 255, 0 , 0);
        break;
      case 3:
      case 4:
        light.setLight(lightTower, 0, 255, 0);
        break;
      case 5:
      case 6:
        light.setLight(lightTower, 0, 0, 255);
        break;
      default:
        light.setLight(lightTower, 255, 255, 0);
        active = 0;
        break;
      }

      if (random(1,101) <= lightMoveChance) {
        lightTower = incColor(lightTower);
      }
    }

    // Fire is queued to the bass channels.  Air effect is random but unlikely right now
     if (currTime > beatWaitTime) {
       if (listenWav.getBeat(bassBand) || listenWav.getBeat(bassBand2)) {
         if (random(1,101) <= beatChance) {
           Serial << F("Fire") << endl;
           hearBeat = true;
           //byte fireLevel = minFirePerFireball / 10 + random(0,maxFirePerFireball / 10);
           byte fireLevel = fscale(0, 100, minFirePerFireball / 10, maxFirePerFireball / 10, random(101), -6.0);
           unsigned long fireMs = fireLevel * 10; // each level is 10ms
           Serial << F(" fireLevel: ") << fireMs;

           flameEffect airEffect = veryRich;

            if (random(1, 101) <= airChance) {
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

            byte towers = random(0,2);

            switch(towers) {
              case 0:
                fire.setFire(fireTower,fireLevel,airEffect);
                break;
              case 1:
                fire.setFire(fireTower,fireLevel,airEffect);
                fire.setFire(oppTower(fireTower),fireLevel,airEffect);
                break;
            }

           network.update();
           fireballs++;
           firepower += fireMs;
           beatEndTime = currTime + fireMs;
           beatWaitTime = currTime + beatInterval;

           fireTower = randColor();
         } else {
           Serial << F("Ignore") << endl;
         }
       }
     }

   }

  light.clear();
  fire.clear();

  // ramp down the volume to exit the music playing cleanly.
  sound.fadeTrack(track);

  Serial << "Fireballs: " << fireballs << " power: " << firepower << " budget: " << budget << endl;
  Serial << F("Gameplay: Player fanfare ended") << endl;

}

void saveFireBudgetFactor(float factor) {
  byte f2b = constrain(factor * 10.0, 0.0, 255.0); // valid cast when factor in [0,25.5]
  EEPROM.write(budgetEepromAddr, f2b);
}

float loadFireBudgetFactor() {
  return( (float)EEPROM.read(budgetEepromAddr)/10.0 );
}


