#ifndef DEPTH_H
#define DEPTH_H

#include <Types.hpp>
#include <Hal/KellerLD.h>
#include <Hal/MS5837.hpp>

using namespace std;

#define NOT_DEFINED -1
#define BAR_100 0
#define BAR_30 1

class Depth
{
public:
    Depth();
    depth getDepth();
    temperature getTemp();
    pressure getPressure();

private:
    KellerLD bar100;
    ms5837 bar30;
    int sensor = NOT_DEFINED;
};

#endif