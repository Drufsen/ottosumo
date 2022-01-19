#include <Arduino.h>
#include "Robot.h"
#include "afstandssensor.h"

AfstandsSensor afstandssensor(13, 12); //VART SKA DENNA VARA?

//public
bool Robot::line_sensors(){
    if(analogRead(A0) < 100 || analogRead(A1) < 100){
        return true;
    return false;
    }
}

int Robot::ultrasonic(){
    int proximity = (afstandssensor.afstandCM());
    return proximity;
}
