//////////////////////////////
//  Please read README.md   //
//////////////////////////////

#include "afstandssensor.h"
#include "Robot.h"

void setup(){
    Serial.begin(9600);
    Serial.println("[completed] setup finished ");
}

void loop(){
    Serial.println(analogRead(A0));
    delay(2000);

    Robot otto = Robot();
    otto.line_sensors();
        
}

//https://towardsdatascience.com/ranking-algorithms-know-your-multi-criteria-decision-solving-techniques-20949198f23e

void check_ultrasonic(){
    if(otto.ultrasonic() < 1000)
        return true;
    else
        return false;
}



void look(){
//called when otto is looking for opponent
  //otto.ultrasonic 

}

void spin(int deg){
  snurra med otto avsedda grader i deg
}
