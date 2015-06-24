
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
    case I_RED: return I_GRN;
    case I_GRN: return I_BLU;
    case I_BLU: return I_YEL;
    case I_YEL: return I_RED;
  }
  
  return I_RED;
  
  // MGD: alternately, this would work (byte cast, increment, modulo, color cast)
//  return( (color) ((byte)val+1 % N_COLORS) );

  // MGD, but I think you really want a clockwise or CCW loop around the towers:
  //
  // GRN > RED
  //  ^     v
  // YEL < BLU
  //
  // clockwise case, you want:
  switch(val) {
    case I_RED: return I_BLU;
    case I_BLU: return I_YEL;
    case I_YEL: return I_GRN;
    case I_GRN: return I_RED;
  }
  return( I_RED ); // as a safety?  
  
}

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
  int track;
  
  if (level == CONSOLATION) {
    track = sound.playWins();
  } else {
    track = sound.playLose();
  }
  
  light.clear();
  fire.clear();
  network.update();

  mic.update();   // populate avg
  mic.update();   // populate avg

  unsigned long trackLength = 30000UL;
  
  switch(level) {
    case LEVEL1:
      trackLength = 7000UL;
      break;
    case LEVEL2:
      trackLength = 12000UL;
      break;
    case LEVEL3:
      trackLength = 20000UL;
      break;
    case LEVEL4:
    case MAXOUT:
      trackLength = 30000UL;
      break;
    case IDLE:
    case CONSOLATION:
      trackLength = 3000UL;
      Serial << "IDLE level: " << trackLength << endl;
      break;
    case NONE:
      return;      
  }
  
  unsigned long startTime = millis() - 1;
   Metro winTime(trackLength);  // TODO: How do we know the length in ms of the track?
   winTime.reset();

   unsigned long beatInterval = 333;  // 180 BPM max
   byte beatChance = 80;  // chance in 100 a beat triggers a fire.  Makes the anim for a specific track different each time
   byte airChance = 10;  // n in 10- chance of air effect
   byte lightMoveChance = 20;  // n in 100 chance of the light moving on a beat
   byte minFirePerFireball = 50;  // min fire level(ms) per fireball
   byte maxFirePerFireball = 200;  // max fire level(ms) per fireball
   
   unsigned long beatEndTime = millis();  // time left for beat effect
   unsigned long beatWaitTime = millis();
   unsigned long currTime;
   int fireballs = 0;
   unsigned long firepower = 1;
   float threshold = 1.0;
   unsigned long budget = (unsigned long) ((float)trackLength / 6.0);
   float bt = (float) trackLength / budget;
   byte active;

  Serial << "Track Length: " << trackLength << " budget: " << budget << endl;;
   
   color fireTower = I_RED;
   color lightTower = I_RED;

   // Use the threshold to meet budget constraints for fire.  Ratio of current time / total Time and fire power / budget.
   while(!winTime.check()) {
     currTime = millis();
     threshold *= bt * (float)firepower / ((float) (currTime - startTime));
     threshold = constrain(threshold,1.0,5.0);
     mic.setThreshold(bassBand, threshold);
     mic.setThreshold(bassBand2, threshold);
     network.update();
     waitDuration(1UL);
     mic.update();

     if (hearBeat && currTime > beatEndTime) {
       Serial << "Beat over.  " << endl;
       light.clear();
       fire.clear();
       network.update();
       hearBeat = false;
       waitDuration(10UL);
     }

    for (byte i = 0; i < NUM_FREQUENCY_BANDS; i++) {
      if ( mic.getBeat(i) ) {
        active++;
      }
    }

    // Lights will queue changes based on activity level across all bands
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
        light.setLight(lightTower, 255, 255, 255);
        active = 0;
        break;
      }

      if (random(0,100) <= lightMoveChance) {
        lightTower = incColor(lightTower);
      }
    }
    
    // Fire is queued to the bass channels.  Air effect is random but unlikely right now
     if (currTime > beatWaitTime) {
       if (mic.getBeat(bassBand) || mic.getBeat(bassBand2)) {
         if (random(0,100) <= beatChance) {
           Serial << "Fire" << endl;
           hearBeat = true;
           byte fireLevel = minFirePerFireball / 10 + random(0,maxFirePerFireball / 10);
           unsigned long fireMs = fireLevel * 10; // each level is 10ms
           
           flameEffect airEffect = veryRich;

            if (random(0, 100) <= airChance) {
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
           fire.setFire(fireTower,fireLevel,airEffect);

           network.update();
           fireballs++;
           firepower += fireMs;
           beatEndTime = currTime + fireMs;
           beatWaitTime = currTime + beatInterval;
           
           fireTower = incColor(fireTower);
         } else {
           Serial << "Ignore" << endl;
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

  
