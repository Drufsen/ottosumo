#include "afstandssensor.h"

class Robot{
    public:
        bool line_sensors();
        int ultrasonic();

        
    private:
        AfstandsSensor afstandssensor(13, 12);
};
