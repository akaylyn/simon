#include "Streaming.h"

void setup() {
  // put your setup code here, to run once:
  randomSeed(A0);
  Serial.begin(115200);
  Serial << "Initialized." << endl;
}

void loop() {
  // put your main code here, to run repeatedly:

//  Serial << trandom(50UL, 300UL, 5000UL) << endl;

  float sum = 0;
  unsigned long tic = millis();
  int Ncomp = 1000;
  for (int i = 0; i < Ncomp; i++) {
    sum += trandom(5000UL, 300000UL, 50000000UL);
  }
  unsigned long toc = millis();
  sum /= Ncomp;

  Serial << endl << "Mean of 10000: " << sum << " should be: " << (5000 + 300000 + 50000000) / 3 << endl << endl;
//  Serial << "trandom took: " << toc - tic << "ms." << " ms/calc: " << float(toc - tic) / float(Ncomp) << endl;
  
  tic = millis();
  sum = 0;
  for (int i = 0; i < Ncomp; i++) {
    sum += sqrt(random(5000UL,50000000UL)*5.0);
  }
  toc = millis();

//  Serial << "sqrt of random took: " << toc - tic << "ms." << " ms/calc: " << float(toc - tic) / float(Ncomp) << endl;
  
  tic = millis();
  sum = 0;
  for (int i = 0; i < Ncomp; i++) {
    sum += random(1,100);
  }
  toc = millis();

//  Serial << "random took: " << toc - tic << "ms." << " ms/calc: " << float(toc - tic) / float(Ncomp) << endl;

  delay(random(500,2000));

}

// generate a random number on the interval [xmin, xmax] with mode xmode.
// takes about 0.22 ms to generate a number.  random(min,max) takes 0.14 ms.  
unsigned long trandom(unsigned long xmin, unsigned long xmode, unsigned long xmax) {
  // using a triangular pdf
  // http://en.wikipedia.org/wiki/Triangular_distribution#Generating_Triangular-distributed_random_variates

  unsigned long modeMinusMin = xmode - xmin;
  unsigned long maxMinusMin = xmax - xmin;
  float cut = modeMinusMin / maxMinusMin;
  // not using random() here: too slow.
  // we'll use the timekeeping function to give a number [0,1] with period 10 ms.
  float u = micros() % 10001 / 10000.; 

  if ( u < cut ) {
    return ( xmin + sqrt( u * maxMinusMin * modeMinusMin ) );
  } else {
    unsigned long maxMinusMode = xmax - xmode;
    return ( xmax - sqrt( (1-u) * maxMinusMin * maxMinusMode ) );
  }
}
