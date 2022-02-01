//////////////////////////////
//  Please read README.md   //
//////////////////////////////

#include "Robot.h"

void setup(){
  //Setup is done within the Robot object's constructor
}

void loop(){
  delay(5000); //We rules state that we need a delay after powerup before the battle begin
  Robot otto = Robot(); //Create the robot object. Constructor preform setup and configuration
  while(true)
    otto.controller(); //Drives the robot based on previous data and sensor input  
}
