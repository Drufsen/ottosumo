#ifndef ROBOT_H //Avoid dubble definition
#define ROBOT_H

// -------- CONFIG ---------
#define TRIG_PIN 13 //Ultrasonic sensor, trigger
#define ECHO_PIN 12 //Ultrasonic sensor, echo

#define LINE_SENSOR_L A0 //Left line sensor input
#define LINE_SENSOR_R A1 //Right line sensor input

#define EN_L_PIN 10 //Enable pin for left motor
#define EN_R_PIN 11 //Enable pin for right motor
#define L_FORWARD_PIN 4 //Left motor forward
#define L_REVERSE_PIN 5 //Left motor reverse
#define R_FORWARD_PIN 2 //Right motor forward
#define R_REVERSE_PIN 3 //Right motor reverse

#define WHITE_BLACK_THRESHOLD 600 //Determens at which value the reading should be intrepreted as black (if value is less than the threshold it is interpreted as white)
#define INVERTED_COLORS // Define this constant if the arena has inverted colors (white center with black borders)
#define MAX_SEARCH_DISTANCE 30 //Defines how far away (in cm) the rebot will look for other robots, a to large value might cause the robot to chase objects outside of the arena
#define STEER_CLEAR_TURN_TIME 500 // Defines how long (and therefor how many degrees) the robot shuld turn when it is manovering away from the border
// ----- END OF CONFIG -----

#include "afstandssensor.h" //To read the ultrasonic sensor

enum direction{stop,forward,back,left,right}; //¨Define a datatype for driving direction makes the code more readable

class Robot{ //The robot is controlled by one object "otto" created by this class
    public:
      Robot(); //Constructor, configure I/O-ports, serial communication etc 
      void controller(); //Drives the robot untill poweroff, we call this function once and it controlls the robot untill power is unplugged
    private:
      //Data members
      AfstandsSensor* ultrasonic; //pointer to object to read the distance to objects in front of the robot using the ultrasonic sensor

      //The following two variables is while the robot is moving away from the border
      unsigned long long steering_clear_of_border; //Stores zero if the robot dose not steer clear of the border at the moment, 
                                             //otherwise it stores a millis value coresponding to the time the robot has completed the clearing border manover
      direction last_know_safe_direction; //Stores the last direction the robot detected was safe to move in

      //The following X variables is used while the robot is searching after the other robot

      //The following X variables is used while the robot is chasing a robot
      
      //Functions
      void drive(direction dir); //Drives the robot in the specified direction dir
      direction check_arena_borders(); //Reads the line sensors returns an direction which the robot can traverse safely, see function for details
      void steer_clear_of_border(); //Manovers the robot away from the border by reversing and making a turn in the direction specified by member last_know_safe_direction
};

#endif // ROBOT_H
