//////////////////////////////
//  Please read README.md   //
//////////////////////////////

#include <afstandssensor.h>
#include "Robot.h"

AfstandsSensor afstandssensor(13, 12); //VART SKA DENNA VARA?

void setup(){
    Serial.begin(9600);
    Serial.println("[completed] setup finished ");
}

void loop(){
    Serial.println(analogRead(A0));
    delay(2000);

    Robot otto = Robot();
    otto.line_sensors();
    otto.ultrasonic();
}
