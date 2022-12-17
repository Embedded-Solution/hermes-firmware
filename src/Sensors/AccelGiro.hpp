#ifndef ACCELGIRO_HPP
#define ACCELGIRO_HPP
#include <Hal/SparkFunLSM9DS1.h>

#define PRINT_CALCULATED
//#define PRINT_RAW
#define PRINT_SPEED 50 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time

// Earth's magnetic field varies by location. Add or subtract
// a declination to get a more accurate heading. Calculate
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.



class AccelGiro 
{
public:
void begin();
void printGyro();
void printAccel();
void printMag();
void printAttitude();
int gyroAvailable();
int accelAvailable();
int magAvailable();
void readGyro();
void readAccel();
void readMag();

private:
LSM9DS1 imu;


};



#endif