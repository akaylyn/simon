#include "Gameplay.h"
#include "Sound.h"

int tower = 2;
int bassBand = 0;
int bassBand2 = 1;
int band2 = 6;
boolean hearBeat = false;
byte active = 0;
int tower2 = 0;

void wait(unsigned long time) {
  Metro delayNow(time);
  
  while(!delayNow.check()) {
    light.update();
  }
}

void playerFanfare(byte level) {
   Serial << "***Inside alan playerFanFare: level: " << level << "\n";

   if (!FANFARE_ENABLED) {
       Serial.println("Fanfare disabled");
       return;
   }

   Serial << F("Gameplay: Player fanfare, level: ") << level << endl;

    //Metro fanfareDuration(FANFARE_DURATION_PER_CORRECT * currentLength);

    // make sweet fire/light/music.
   int winTrack = sound.playWins();
    
    
    light.show();
//   Metro winTime(5000 * level);  // TODO: How do we know the length in ms of the track?
   
   mic.update();   // populate avg
   mic.update();   // populate avg
   
   float beatThreshold = 3;
   byte fireLevel = 2;  // power curve of fire level mapping, higher increases average fire level
   byte airChance = 0;  // n in 10 chance of air effect
   int fireBeat = 450;
   
   // stats
   int fireballs = 0;
   int firepower = 0;

   mic.setThreshold(bassBand,beatThreshold);
   mic.setThreshold(bassBand2,beatThreshold);

   Metro winTime(30000);  // TODO: How do we know the length in ms of the track?
   winTime.reset();
   
   Metro beatWait(333);
   
/*
   while(!winTime.check()) {
     wait(1);
     mic.update();
     Serial << "vol: " << mic.getVol(bassBand) << " " << mic.getVol(bassBand2) << " avg: " << mic.getAvg(bassBand) << " " << mic.getAvg(bassBand2) << " beat: " << mic.getBeat(bassBand) << " " << mic.getBeat(bassBand2) << endl;
     if ((mic.getBeat(bassBand) || mic.getBeat(bassBand2)) && hearBeat == false) {
     }  
     wait(10);
   }
*/   
   while(!winTime.check()) {
     wait(1);
     mic.update();

      // clear packet  
      light.setAllLight(0);
      light.setAllFire(0);
      
      for(byte i = 0; i<NUM_FREQUENCY_BANDS; i++) {
         if( mic.getBeat(i) ) {
           active++;
         }
      }
   
//      if ((mic.getBeat(bassBand) || mic.getBeat(bassBand2)) && hearBeat == false) {
      if (beatWait.check() && (mic.getVol(bassBand) > fireBeat || mic.getVol(bassBand2) > fireBeat) && hearBeat == false) {
        tower++;
        if (tower > 5) tower = 2;
        byte avgVol = max(mic.getAvg(bassBand),mic.getAvg(bassBand2));
//        Serial << "avgVol: " << avgVol << " fscale: " << fscale(0, 1024, 0, 255, avgVol, fireLevel) << endl;
        //Serial << "band1: " << mic.getAvg(bassBand) << " band2: " << mic.getAvg(bassBand) << endl;
        
        byte fire = floor(fscale(0, 1024, 50, 255, avgVol, fireLevel));
        flameEffect_t airEffect = FE_veryRich;
        
        if (random(0,10) >= (10-airChance)) {
          byte effect = random(0,6);
          switch(effect) {
            case 0:
              airEffect = FE_kickStart;
              break;  
            case 1:
              airEffect = FE_kickMiddle;
              break;  
            case 2:
              airEffect = FE_kickEnd;
              break;  
            case 3:
              airEffect = FE_gatlingGun;
              break;  
            case 4:
              airEffect = FE_randomly;
              break;  
            case 5:
              airEffect = FE_veryLean;
              break;   
          }             
        }
        //Serial << "Fireballv3: " << (fire) << " air: " << (airEffect) << endl;
        light.setFire(tower-2,fire,airEffect,true,tower);
        hearBeat = true;
        fireballs++;
        firepower += fire;
        beatWait.interval(333);
        beatWait.reset();
        
//      } else if (!(mic.getBeat(bassBand) || mic.getBeat(bassBand2))) {
      } else if (!(mic.getVol(bassBand) > fireBeat || mic.getVol(bassBand2) > fireBeat)) {
        //light.setAllFire(0);  // TODO: Not sure we really want to shut off flame here
        //light.show();
        hearBeat = false;
      }
        // clear packet  
      light.setAllLight(0);
      light.setAllFire(0);
    
    if (active > 0) {
       switch(active) {
         case 0:
           light.setAllLight(0);
           break;
         case 1:
         case 2:
           light.setLight(I_RED,255);
           break;
         case 3:
         case 4:
           light.setLight(I_GRN,255);
           break;
         case 5:
         case 6:
           light.setLight(I_BLU,255);
           break;
         default:
           light.setAllLight(255);
           active = 0;
           break;
       }
           
       light.show(tower2);
       if (random(10) > 2) {
          tower2++;
       }
       if (tower2 > 6) tower2 = 0;
    }
   }  

   light.setAllLight(LIGHT_OFF);
   light.setAllFire(LIGHT_OFF);

    // ramp down the volume to exit the music playing cleanly.
    sound.fadeTrack(winTrack);

    Serial << "Fireballs: " << fireballs << " power: " << firepower << endl;
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

