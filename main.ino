//////////////////////////////
//  Please read README.md   //
//////////////////////////////

#include <afstandssensor.h>
#include "Robot.h"

AfstandsSensor afstandssensor(13, 12);

void setup(){
    Serial.begin(9600);
    Serial.println("[completed] setup finished ");

}

void loop(){

    //Serial.println(afstandssensor.afstandCM());
    Serial.println(analogRead(A0));
    delay(2000);
}


void check_line(){
    if(line_sensor_1 || line_sensor_2){





    }
}

//sensor data
bool line_sensor_1(){
    if(analogRead(A0) < 100){
        return true;
    return false;
    }
}

bool line_sensor_2(){
    if(analogRead(A1) < 100){
        return true;
    return false;
    }
}
