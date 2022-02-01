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
  this->last_know_safe_direction = forward; //At the start it is assumed to be safe to move forward (since the robot start in the middle of the arena) 

  randomSeed(ultrasonic->afstandCM()); //Configure random numbers
}

void Robot::controller() { //Drives the robot based on previous data and sensor input
  
  switch(this->driving_mode) {
    case('h'): //In case the robot is halted, used while debugging
      this->drive(stop); //Stop the robot        
      break; //Do not do anything else until otherwise instructed, this will cause an infinite loop which repeatedly stops the motors
    
    case('b'): //In case we are avoiding the border
      steer_clear_of_border(); //Continue to steer clear of border 
      break; //Do not do anything else until clearing maneuver is done
    
    default: //If none of the above cases are true we are either searching or following a robot
      if(!is_borders_clear()) //Check if the borders is clear, if they aren't...
        break; //break to make the program loop back to top of switch statement and re-evaluate the situation
      
      //If the program got to this line the robot is safe to search or follow other robots
      if(this->driving_mode == 'f') //If the robot is following a robot
        this->follow(); //Continue to follow
      else //Else, the only remaining driving mode is search
        this->search(); //Continue to search
  }
  this->sensor_debugging(); //For debugging, prints debugging info over Serial interface
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

bool Robot::is_borders_clear() { //Reads the line sensors and returns true if no borders is found, otherwise false
                                 //If boarder is detected the driving mode is changed to 'b' boarder avoidance mode
  /* Value of last_know_safe_direction member explained:
   *   Forward - no borders detected, the robot can safely travel in all directions
   *   Left - border detected on the right side, the robot needs to make a left turn
   *   Right - border detected on the left side, the robot needs to make a right turn
   */ 

  bool l_border_detected = analogRead(LINE_SENSOR_L) < WHITE_BLACK_THRESHOLD; //If left sensor vlaue is less than threshold value the sensor reads white meaning we are at the border (and need to turn)
  bool r_border_detected = analogRead(LINE_SENSOR_R) < WHITE_BLACK_THRESHOLD; //If right sensor vlaue is less than threshold value the sensor reads white meaning we are at the border
  
  #ifdef INVERTED_COLORS //If the arena has inverted colours the invert the "side_clear" variables
  l_border_detected = !l_border_detected;
  r_border_detected = !r_border_detected;
  #endif //INVERTED_COLORS
  
  if(l_border_detected) //Border att left side...
    this->last_know_safe_direction = right; //... safe direction is right
  else if(r_border_detected)
    this->last_know_safe_direction = left;
  else { //No borders was detected
    this->last_know_safe_direction = forward; //It is safe to move forward and all other directions
    return true; //We did not detect any borders
  }
  //If the program gets to this line, we did detect borders
  this->driving_mode = 'b'; //Set new driving mode
  this->search_timer = 0; //Make sure that the search function is reset, this is needed in case we detect the border while searching
  this->follow_timer = 0; //Make sure that the follow function is reset, this is needed in case we detect the border while following a robot
  return false;
}

void Robot::steer_clear_of_border() { //Manovers the robot away from the border by reversing and making a turn in the direction specified by dir
  if(this->reverse_finish_time == 0) { //If zero, we have not yet determined timing for maneuver
    this->reverse_finish_time = millis() + STEER_CLEAR_REVERSE_TIME; //Set reverse finish time based on config parameter
    float noise = random(11)/20.0 + 0.75; //Generate a random number between 0.75 and 1.25 (inclusive)
    this->turn_finish_time = this->reverse_finish_time + STEER_CLEAR_TURN_TIME*noise ; //Set turn finish time based on config parameter + noise to make the robot more unpredictable
  }
  else if(millis() > this->turn_finish_time) { //If current time is greater than finish time
    this->driving_mode = 's'; //We are done clearing border and can start search for opponent
    this->reverse_finish_time = 0; //Set this to zero so that the function knows that timers need to be set next time we need to clear the border
    return; //Go back to main control function
  }
  //If the program get to this line we are currently preforming the  maneuver
  if(millis() < this->reverse_finish_time) //If we are in the reverse phase of maneuver
    this->drive(back); //Reverse
  else //Otherwise we are in the turn phase
    this->drive(this->last_know_safe_direction); //Turn in the last known safe direction
}

void Robot::search() { //Searches for other robots in the arena
  if(this->search_timer == 0) { //If zero we have not configured the search timer
   this->search_timer = millis()+ MAX_SEARCH_DRIVE_TIME; //Set max rotate time
   this->search_mode_rotate = false; //Set rotate mode
   this->drive(forward); //Start driving forward 
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
  
  if(this->search_mode_rotate && ultrasonic->afstandCM() < MAX_SEARCH_DISTANCE && ultrasonic->afstandCM() < MAX_SEARCH_DISTANCE) { //If we find a robot (it will only look while rotating), it will perform two measurements to lower the risk of false trigger                                                                            //because it will need to know which side of the robot i detected, see follow function for details)
    driving_mode = 'f'; //Set driving mode to follow
    this->search_timer = 0; //To let the function know that we are starting a new search next time it is called
  }
}

void Robot::follow() { //Follows and pushes other robots out of the arena
  /* Folllow and push algorimt:
   *  1. Starting point, the search found the left corner of another robot (left corner detected since always rotate clockwise)
   *  2. Now go forward and a little to the right until the corner of the robot disappears (the sensor reads a large value)
   *  3. Go forward and a little to the right until the corner of the robot appears (the sensor reads a shorter distance)
   *  4. Repeat step 2 and 3 until we push opponent out of the arena (indicated by the fact that we get to the border)
   *  5. The border function will change mode to 'b'
   *  If the robot misses or fails to hit the opponent it will hit the border and the 'b' mode will activate or the timeout timer will return the robot to 's' mode. 
   */
  if(this->follow_timer == 0) //If zero we have not configured the follow timer
    this->follow_timer = millis() + FOLLOW_MAX_TIME; //Set new timer by specified config parameter
  else if (this->follow_timer < millis()) { //If max time is passed
    this->follow_timer = 0; //Reset timer to zero so that the function knows that it should start a new timer next time it is called
    this->driving_mode = 's'; //Return to search mode
  }

  if(ultrasonic->afstandCM() < MAX_SEARCH_DISTANCE + 15) //If we see opponents left corner (we add a little to the scanning distance due to measurement inaccuracy and opponent’s movements)
    drive(forward, L_MOTOR_SPEED, R_MOTOR_SPEED/(SLOW_SPEED_FACTOR*2)); //Steer a little to the left aim left of the robot
  else //We do not see the robot
    drive(forward, L_MOTOR_SPEED/(SLOW_SPEED_FACTOR*2), R_MOTOR_SPEED); //Steer a little to the right so that we align with the left corner again
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
