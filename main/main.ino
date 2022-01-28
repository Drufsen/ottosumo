//////////////////////////////
//  Please read README.md   //
//////////////////////////////

#include "afstandssensor.h"
#include "Robot.h"

AfstandsSensor afstandssensor(13, 12);

void setup(){
    Serial.begin(9600);
    Serial.println("[completed] Setup finished ");
}

void loop(){
    Serial.println(analogRead(A0));
    delay(2000);

    Robot otto = Robot();
    otto.line_sensors();
        
}

//https://towardsdatascience.com/ranking-algorithms-know-your-multi-criteria-decision-solving-techniques-20949198f23e

//void check_ultrasonic(){
//    
//        return true;
//    else
//        return false;
//}



void look(){
//called when otto is looking for opponent
    spin(360, 1);
    if(otto.ultrasonic() < 1000)
    
}

void spin(int deg,int cmd){ //cmd: 1=start, 0=stop
    if(cmd){
        //motor_right deg
        return;
    //motor stop;
    }
}
