#include "Gameplay.h"
#include "Sound.h"

int tower = 2;
int band = 1;
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

//  mic.setThreshold(band,5);
//  mic.setThreshold(band2,5);
  mic.setThreshold(band,2);
  mic.setThreshold(band2,2);
    Serial << F("Gameplay: Player fanfare, level: ") << level << endl;

    //Metro fanfareDuration(FANFARE_DURATION_PER_CORRECT * currentLength);

    // make sweet fire/light/music.
    int winTrack = sound.playWins();
    
    
   light.show();
//   Metro winTime(5000 * level);  // TODO: How do we know the length in ms of the track?
   Metro winTime(30000);  // TODO: How do we know the length in ms of the track?
    
   while(!winTime.check()) {
     mic.update();

      // clear packet  
      light.setAllLight(0,false,0);
      light.setAllFire(0,false,0);
      
      for(byte i = 0; i<NUM_FREQUENCY_BANDS; i++) {
         if( mic.getBeat(i) ) {
           active++;
         }
      }
      
      if (mic.getBeat(band) && hearBeat == false) {
        tower++;
        if (tower > 5) tower = 2;
        //light.setLight(I_BLU,255,false,tower);
//        light.setFire(I_RED,(int)((mic.getAvg(band)/1024.0)*255),false,tower);
        byte fire = 50 + (int)((mic.getAvg(band)/1024.0)*205); 
        Serial << "Fireballv3: " << (fire) << " avg: " << (mic.getAvg(band)) << endl;
        //light.setFire(I_RED,(int)((mic.getAvg(band)/1024.0)*255),false);
//        light.setFire(I_RED,fire,false);
        light.setAllFire(fire,false,0);
//        light.show(tower);
        light.show();
        hearBeat = true;
      } else if (!mic.getBeat(band)) {
//        light.setFire(I_RED,0,false,tower);
//        light.setFire(I_RED,0,false);
        light.setAllFire(0,false,0);
//        light.show(tower);
        light.show();
        hearBeat = false;
      }
        // clear packet  
      light.setAllLight(0,false,0);
      light.setAllFire(0,false,0);
    
    if (active > 0) {
       switch(active) {
         case 0:
           light.setAllLight(0,false,0);
           break;
         case 1:
         case 2:
           light.setLight(I_RED,255,false,0);
           break;
         case 3:
         case 4:
           light.setLight(I_GRN,255,false,0);
           break;
         case 5:
         case 6:
           light.setLight(I_BLU,255,false,0);
           break;
         default:
           light.setAllLight(255,false,0);
           active = 0;
           break;
       }
           
       light.show(tower2);
       if (random(10) > 2) {
          tower2++;
       }
       if (tower2 > 6) tower2 = 0;
    }
    wait(1); 
   }  

   light.setAllLight(LIGHT_OFF);
   light.setAllFire(LIGHT_OFF);

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

