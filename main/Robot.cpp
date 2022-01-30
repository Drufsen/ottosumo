#include "Robot.h"

#include <Arduino.h>

Robot::Robot() { //Constructor, configure I/O-ports, serial communication, and data members 
  Serial.begin(115200); //Init. serial communication, used for debugging and monitoring
  
  //Configure IO
  pinMode(EN_L_PIN, OUTPUT);
  pinMode(EN_R_PIN, OUTPUT);
  pinMode(L_FORWARD_PIN, OUTPUT);
  pinMode(L_REVERSE_PIN, OUTPUT);
  pinMode(R_FORWARD_PIN, OUTPUT);
  pinMode(R_REVERSE_PIN, OUTPUT);

  analogWrite(EN_L_PIN, L_MOTOR_SPEED ); //Enable left motor
  analogWrite(EN_R_PIN, R_MOTOR_SPEED ); //Enable right motor
  this->drive(stop); //Make sure the robot is stationary

  //Set data members
  this->ultrasonic = new AfstandsSensor(TRIG_PIN, ECHO_PIN); //Prepare ultrasonic sensor
  this->driving_mode = 's'; //Start the robot in search mode
  this->last_know_safe_direction = forward; //At the start it is asumed to be safe to move forward (since the robot start in the middle of the arena) 

  randomSeed(ultrasonic->afstandCM()); //Configure random numbers
}

void Robot::controller() { //Drives the robot based on previous data and sesnor input
  
  switch(this->driving_mode) {
    case('h'): //In case the robot is halted
      this->drive(stop); //Stop the robot        
      break; //Do not do anything else untill othervise instructed, this will cuase an infinite loop which repeatedly stops the motors
    
    case('b'): //In case we are avoiding the border
      steer_clear_of_border(); //Continue to steer clear of border 
      break; //Do not do annything else untill clearing manover is done
    
    default: //If none of the above cases are true we are either searching or following a robot
      if(!is_borders_clear()) //Check if the borders is clear, if they aren't...
        break; //break to make the program loop back to top of switch statment and reevaluate the situation
      
      //If the program got to this line the robot is safe to search or follow other robots
      if(this->driving_mode == 'f') //If the robot is following a robot
        this->follow(); //Continue to follow
      else //Else, the only remaining driving mode is search
        this->search(); //Continue to search
  }
  this->sensor_debugging();
}

void Robot::drive(direction dir, short left_speed = L_MOTOR_SPEED, short right_speed = R_MOTOR_SPEED){ //Drives the robot in the specified direction dir with the speeds specified by the two other parameters
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
  analogWrite(EN_L_PIN, left_speed ); //Set speed on left motor
  analogWrite(EN_R_PIN, right_speed); //Set speed on right motor
}

bool Robot::is_borders_clear() { //Reads the line sensors and returns true if no borders is foud, otherwise false
  /* Value of last_know_safe_direction member explained:
   *   Forward - no borders detected, the robot can safly travel in all directions
   *   Left - border deteced on the right side, the robot needs to make a left turn
   *   Right - border deteced on the left side, the robot needs to make a right turn
   */ 

  bool l_border_detected = analogRead(LINE_SENSOR_L) < WHITE_BLACK_THRESHOLD; //If left sensor vlaue is less than threshold value the sensor reads white meaning we are at the border (and need to turn)
  bool r_border_detected = analogRead(LINE_SENSOR_R) < WHITE_BLACK_THRESHOLD; //If right sensor vlaue is less than threshold value the sensor reads white meaning we are at the border
  
  #ifdef INVERTED_COLORS //If the arena has inverted colors the invert the "side_clear" variables
  l_border_detected = !l_border_detected;
  r_border_detected = !r_border_detected;
  #endif //INVERTED_COLORS
  
  if(l_border_detected) //Border att left side...
    this->last_know_safe_direction = right; //... safe direction is right
  else if(r_border_detected)
    this->last_know_safe_direction = left;
  else { //No borders was detected
    this->last_know_safe_direction = forward; //It is safe to move forward and all other directions
    return true; //We did not detect anny borders
  }
  //If the program get to this line, we did detect borders
  this->driving_mode = 'b'; //Set new driving mode
  this->search_timer = 0; //Make sure that the search function is reset, this is needed in case we detect the border while searching
  this->follow_timer = 0; //Make sure that the follow function is reset, this is needed in case we detect the border while following a robot
  return false;
}

void Robot::steer_clear_of_border() { //Manovers the robot away from the border by reversing and making a turn in the direction specified by dir
  if(this->reverse_finish_time == 0) { //If zero, we have not yet determened timing for manover
    this->reverse_finish_time = millis() + STEER_CLEAR_REVERSE_TIME; //Set reverse finish time based on config parameter
    float noise = random(11)/20.0 + 0.75; //Generate a random number between 0.75 and 1.25 (inclusive)
    this->turn_finish_time = this->reverse_finish_time + STEER_CLEAR_TURN_TIME*noise ; //Set turn finish time based on config parameter + noise to make the robot more unpredictable
  }
  else if(millis() > this->turn_finish_time) { //If current time is greater than finish time
    this->driving_mode = 's'; //We are done clering border and can start search for oponent
    this->reverse_finish_time = 0; //Set this to zero so that the function knows that timers need to be set next time we need to clear the border
    return; //Go back to main control function
  }
  //If the program get to this line we are currently preforming the  manover
  if(millis() < this->reverse_finish_time) //If we are in the reverse phase of manover
    this->drive(back); //Reverse
  else //Otherwise we are in the turn phase
    this->drive(this->last_know_safe_direction); //Turn in the last known safe direction
}

void Robot::search() { //Searches for other robots in the arena
  if(this->search_timer == 0) { //If zero we have not configured the search timer
   this->search_timer = millis()+ MAX_SEARCH_DRIVE_TIME; //Set max rotate time
   this->search_mode_rotate = false; //Set rotate mode
   this->drive(forward); //Start drving forward 
  }
  else if(this->search_timer < millis() && this->search_mode_rotate) { //If max time is passed and we are in rotate mode
     this->search_mode_rotate = false; //Set rotate mode false, to make robot find new location to search in
     this->search_timer = millis()+ MAX_SEARCH_DRIVE_TIME; //Set time when the robot should start circeling agin
     this->drive(forward); //Go to new location
  }
  else if(this->search_timer < millis() && !this->search_mode_rotate) { //If max time is passed and we are in "go to new location mode"
    this->search_mode_rotate = true; //Set rotate mode true
    this->search_timer = millis()+ MAX_SEARCH_ROTATE_TIME; //Set max time for rotation
    this->drive(right, L_MOTOR_SPEED/SLOW_SPEED_FACTOR, R_MOTOR_SPEED/SLOW_SPEED_FACTOR); //Start rotating
  }
  
  if(this->search_mode_rotate && ultrasonic->afstandCM() < MAX_SEARCH_DISTANCE && ultrasonic->afstandCM() < MAX_SEARCH_DISTANCE) { //If we find a robot (it will only look while rotating), it will preform two messurments to lower the risk of false triger                                                                            //because it will need to know which side of the robot i detected, see follow function for details)
    driving_mode = 'f'; //Set driving mode to follow
    this->search_timer = 0; //To let the function know that we are starting a new search next time it is called
  }
}

void Robot::follow() { //Follows and pushes other robots out of the arena
  /* Folllow and push algorimt:
   *  1. Starting point, the search found the left corner of another robot (left corner detected since always rotate clockwise)
   *  2. Now go forward and a little to the right untill the corner of the robbot disappears (the sensor reads a large value)
   *  3. Go forward and a litle to the right untill the corner of the robbot appears (the sensor readsa shorter distance)
   *  4. Repeat step 2 and 3 untill  we push oponent out of the arena (indicated by the fact that we get to the border)
   *  5. The border function will change mode to 'b'
   *  If the misses or fails to hit the oponent it will hit the border and the 'b' mode will activate or the timeout timer will return the robot to 's' mode. 
   */
  if(this->follow_timer == 0) //If zero we have not configured the follow timer
    this->follow_timer = millis() + FOLLOW_MAX_TIME; //Set new timer by specifed config parameter
  else if (this->follow_timer < millis()) { //If max time is passed
    this->follow_timer = 0; //Reset timer to zero so that the function knows that it should start a new timer next time it is called
    this->driving_mode = 's'; //Return to search mode
  }

  if(ultrasonic->afstandCM() < MAX_SEARCH_DISTANCE + 15) //If we see oponents left corner (we add a little to the scaning distance due to mesurmnet inaccuracy and oponent's movemnts)
    drive(forward, L_MOTOR_SPEED, R_MOTOR_SPEED/(SLOW_SPEED_FACTOR*2)); //Steer a little to the left aim left of the robot
  else //We do not see the robot
    drive(forward, L_MOTOR_SPEED/(SLOW_SPEED_FACTOR*2), R_MOTOR_SPEED); //Steer a little to the right so that we align with the left corner agin
}

void Robot::sensor_debugging() { //Prints sensor values to serial monitor
  Serial.print("Mode: ");
  Serial.print(this->driving_mode);
  Serial.print(" Left: ");
  Serial.print(analogRead(LINE_SENSOR_L));
  Serial.print(" Right: ");
  Serial.print(analogRead(LINE_SENSOR_R));
  Serial.print(" Dist: ");
  Serial.print(ultrasonic->afstandCM());
  Serial.println(" cm");
}
