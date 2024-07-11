#include <Arduino.h>
#include <DallasTemperature.h>
#include "PCC_V4.h"

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}
//hhgiugiugiugtiugtufruyfdruyfdry
void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
  Serial.println("MY PCC");
}