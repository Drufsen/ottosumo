#include "Robot.h"

#include <Arduino.h>

Robot::Robot() { //Constructor, configure I/O-ports, serial communication, and data members 
  Serial.begin(115200); //Init. serial communication, used for debugging and monitoring
  
  //Set data members
  this->steering_clear_of_border = 0; //Zero menas the robot is not clearing the border
  this->last_know_safe_direction = forward; //At the start the it is asumed to be safe to move forward (since the robot start in the middle of the arena) 
  this->ultrasonic = new AfstandsSensor(TRIG_PIN, ECHO_PIN); //Prepare ultrasonic sensor
  
  //Configure IO
  pinMode(EN_L_PIN, OUTPUT);
  pinMode(EN_R_PIN, OUTPUT);
  pinMode(L_FORWARD_PIN, OUTPUT);
  pinMode(L_REVERSE_PIN, OUTPUT);
  pinMode(R_FORWARD_PIN, OUTPUT);
  pinMode(R_REVERSE_PIN, OUTPUT);

  analogWrite(EN_L_PIN, 255); //Enable left motor
  analogWrite(EN_R_PIN, 255); //Enable right motor
  this->drive(stop); //Make sure the robot is stationary
}

void Robot::controller() { //Drives the robot untill poweroff, we call this function once and it controlls the robot untill power is unplugged

  while(true) {
    /* While driving the robot folows a predefined algoritm to decide were to go:
     *  
     */
  if(steering_clear_of_border != 0) //If we are steering clear of the border
    steer_clear_of_border(); //Continue to steer away from boarder
  else { //We are currently not steering away from any borders
    
    direction safe_dir = this->check_arena_borders();
    if(safe_dir != forward) { //We are at the border and need to steer clear of it!
      this->steering_clear_of_border = millis() + STEER_CLEAR_TURN_TIME; //Indicate to progarm that we currently are manovering away from a border from this time untill "current time + STEER_CLEAR_TURN_TIME ms"
      this->last_know_safe_direction = safe_dir; //Store the direction we need to turn to avoid boarder
    }
    else { //We are safe to continue forward/chase other robot/search for robot
      drive(forward);
    }
  }
   
   
    
  }
}

void Robot::drive(direction dir){ //Drives the robot in the specified direction dir
  switch(dir) {
    case(0): //Stop
      digitalWrite(L_FORWARD_PIN,LOW);
      digitalWrite(L_REVERSE_PIN,LOW);
      digitalWrite(R_FORWARD_PIN,LOW);
      digitalWrite(R_REVERSE_PIN,LOW);
      break;
    case(1): //Forward
      digitalWrite(L_FORWARD_PIN,HIGH);
      digitalWrite(L_REVERSE_PIN,LOW);
      digitalWrite(R_FORWARD_PIN,HIGH);
      digitalWrite(R_REVERSE_PIN,LOW);
      break;
    case(2): //Back
      digitalWrite(L_FORWARD_PIN,LOW);
      digitalWrite(L_REVERSE_PIN,HIGH);
      digitalWrite(R_FORWARD_PIN,LOW);
      digitalWrite(R_REVERSE_PIN,HIGH);
      break;
    case(3): //Left
      digitalWrite(L_FORWARD_PIN,LOW);
      digitalWrite(L_REVERSE_PIN,HIGH);
      digitalWrite(R_FORWARD_PIN,HIGH);
      digitalWrite(R_REVERSE_PIN,LOW);
      break;
    case(4): //Right
      digitalWrite(L_FORWARD_PIN,HIGH);
      digitalWrite(L_REVERSE_PIN,LOW);
      digitalWrite(R_FORWARD_PIN,LOW);
      digitalWrite(R_REVERSE_PIN,HIGH);
  }
}

direction Robot::check_arena_borders() { //Reads the line sensors returns an direction which the robot can traverse safely.
  /* Return value of:
   *   Forward - no borders detected, the robot can safly travel in all directions
   *   Left - border deteced on the right side, the robot needs to make a left turn
   *   Right - border deteced on the left side, the robot needs to make a right turn
   */ 

  bool l_border_detected = analogRead(LINE_SENSOR_L) < WHITE_BLACK_THRESHOLD; // If left sensor vlaue is less than threshold value the sensor reads white meaning we are at the border (and need to turn)
  bool r_border_detected = analogRead(LINE_SENSOR_R) < WHITE_BLACK_THRESHOLD; // If right sensor vlaue is less than threshold value the sensor reads white meaning we are at the border

  Serial.print("Left: ");
  Serial.print(analogRead(LINE_SENSOR_L));
  Serial.print("    Right: ");
  Serial.println(analogRead(LINE_SENSOR_R));
  
  #ifdef INVERTED_COLORS // If the arena has inverted colors the invert the "side_clear" variables
  l_border_detected = !l_border_detected;
  r_border_detected = !r_border_detected;
  #endif // INVERTED_COLORS
  direction dir = forward; //Stores the direction the robot can traverse safely
  if(l_border_detected) //Border att left side...
    dir = right; //... safe direction is right
  else if(r_border_detected)
    dir = left;
  
  return dir;
}

void Robot::steer_clear_of_border() { //Manovers the robot away from the border by reversing and making a turn in the direction specified by dir
  if(this->steering_clear_of_border > millis()) { //We are still steering cler of the boarder
    drive(this->last_know_safe_direction); //Steer in the safe direction
  }
  else { //We are done steering away from border, because the current time has passed the end time in steering_clear_of_border
    this->steering_clear_of_border = 0; //Indicate tp program that we are done clearing the boarder
  }
}
