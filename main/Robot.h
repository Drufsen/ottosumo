#ifndef ROBOT_H //Avoid dubble definition
#define ROBOT_H

// ######### CONFIG #########
// --------   I/O  ----------
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

// ---- DRIVE PARAMETERS ----
#define L_MOTOR_SPEED 255 //Left motor speed, value between 0 (stopped) - 255 (max)
#define R_MOTOR_SPEED 255 //Right motor speed, value between 0 (stopped) - 255 (max)
#define STEER_CLEAR_REVERSE_TIME 400 //Defines how long in ms (and therefor how far) the robot should reverse while clearing the border
#define STEER_CLEAR_TURN_TIME 400 //Defines how long in ms (and therefor how many degrees) the robot should turn when it is maneuver away from the border
#define SLOW_SPEED_FACTOR 3 //Sometimes one of or both wheels will drive in slow mode, in this case the speed will be X_MOTOR_SPEED/SLOW_SPEED_FACTOR

// ---------- MISC ----------
#define WHITE_BLACK_THRESHOLD 300 //determines at which value the reading should be intrepreted as black (if value is less than the threshold it is interpreted as white)
//#define INVERTED_COLORS //Define this constant if the arena has inverted colours (white arena with black borders)
#define MAX_SEARCH_DISTANCE 40 //Defines how far away (in cm) the robot will look for other robots, a to large value might cause the robot to chase objects outside of the arena
#define MAX_SEARCH_ROTATE_TIME 3000 //Defines how long in ms the robot will rotate before going to a new location
#define MAX_SEARCH_DRIVE_TIME 1000 //Defines how long in ms the robot will drive when going to a new search location
#define FOLLOW_MAX_TIME 7000 //Defines how long in ms the robot will follow another robot before timing out and returning to search mode
// ##### END OF CONFIG #####

#include "afstandssensor.h" //To read the ultrasonic sensor

enum direction{stop,forward,back,left,right}; //¨Define a datatype for driving direction makes the code more readable

class Robot{ //The robot is controlled by one object "otto" created by this class
  public:
    Robot(); //Constructor, configure I/O-ports, serial communication etc 
    void controller(); //Drives the robot based on previous data and sensor input
  private:
    //Functions
    void drive(direction dir, short left_speed = L_MOTOR_SPEED, short right_speed = R_MOTOR_SPEED); //Drives the robot in the specified direction dir with the speeds specified by the two other parameters
    bool is_borders_clear(); //Reads the line sensors and returns true if no borders is found, otherwise false
    void steer_clear_of_border(); //Maneuvers the robot away from the border by reversing and making a turn in the direction specified by member last_know_safe_direction
    void search(); //Searches for other robots in the arena
    void follow(); //Follows and pushes other robots out of the arena
    void sensor_debugging(); //Prints the sensor values to serial monitor 

    //Data members
    AfstandsSensor* ultrasonic; //pointer to object to read the distance to objects in front of the robot using the ultrasonic sensor
    char driving_mode; //Stores the mode the robot is in: 'h' = halted stopped, 'b' = avoiding Border, 's' = searching, 't' = tracking opponent
    
    //Data members used while steering clear of border
    direction last_know_safe_direction; //Stores the last direction the robot detected was safe to move in
    unsigned long long reverse_finish_time = 0; //Stores the time in ms when the robot has completed the reversing away from the border
    unsigned long long turn_finish_time = 0; //Stores the time in ms when the robot has completed the turn to face away from border
    
    //Data members used while searching for other robot
    unsigned long long search_timer = 0; //Used to keep track of how long the robot has been rotating or driving while searching for other robot
    bool search_mode_rotate; //Used to store the current mode of search, either the robot is rotating and looking around (if true) or driving forward to a new search location (f false)

    //Data members used while following the other robot
    unsigned long long follow_timer = 0; //Used to keep track of how long the robot has been following the other robot
};

#endif //ROBOT_H
