//////////////////////////////
//  Please read README.md   //
//////////////////////////////

#include "Robot.h"

void setup(){
  //Setup is done within the Robot object's constructor
}

void loop(){
  Robot otto = Robot(); //Create the robot object. Constructor preform setup and configuration
  otto.controller(); //This function controlls the robot during the compatition, it will run untill power is disconnected     
}
