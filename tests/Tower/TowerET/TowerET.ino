// need to strike RobotIRRemote directory in Arduino IDE libraries folder.  Name collision!
#include <Streaming.h>
#include <Metro.h>
#include <Simon_Common.h> // I_RED, etc.
#include <EasyTransfer.h> // rx, tx
#include <SoftwareSerial.h> // 

SoftwareSerial SSerial(A2, A3); // 
EasyTransfer ET; 
colorInstruction newColorInst;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial << "Startup." << endl;
  
  SSerial.begin(9600);
  ET.begin(details(newColorInst), &SSerial);

}

void loop() {
  // put your main code here, to run repeatedly: 
  newColorInst = cRed;
  ET.sendData();
  delay(3000);

  newColorInst = cOff;
  ET.sendData();
  delay(1000);

  newColorInst = cGreen;
  ET.sendData();
  delay(3000);

  newColorInst = cOff;
  ET.sendData();
  delay(1000);

  newColorInst = cBlue;
  ET.sendData();
  delay(3000);

  newColorInst = cOff;
  ET.sendData();
  delay(1000);

  newColorInst = cYellow;
  ET.sendData();
  delay(3000);

  newColorInst = cOff;
  ET.sendData();
  delay(1000);

  newColorInst = cWhite;
  ET.sendData();
  delay(3000);

  newColorInst = cOff;
  ET.sendData();
  delay(1000);

}
