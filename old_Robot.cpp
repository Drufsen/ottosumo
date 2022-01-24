#include <Arduino.h>
#include "Robot.h"
#include "afstandssensor.h"

//public
bool Robot::line_sensors(){
    if(analogRead(A0) < 100 || analogRead(A1) < 100){
        return true;
    return false;
    }
}

int Robot::ultrasonic(){
    int proximity = (this.afstandssensor.afstandCM());
    return proximity;
}
