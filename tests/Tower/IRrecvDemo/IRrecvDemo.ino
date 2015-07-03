/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>
#include <Streaming.h>

int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setup()
{
  Serial.begin(115200);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial << "Key (0x" << _HEX((results.value << 16) >> 24) << ") ";
    Serial << "Address (0x" << _HEX(results.value >> 16) << ") ";
    Serial << "Code (0x" << _HEX(results.value) << ") ";
    Serial << "Raw length(" << results.rawlen << ") ";
    Serial << endl;
    irrecv.resume(); // Receive the next value
  }
}
