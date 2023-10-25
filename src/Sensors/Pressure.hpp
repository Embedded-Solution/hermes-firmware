#ifndef PRESSURE_HPP
#define PRESSURE_HPP

#include <Types.hpp>

using namespace std;

class Pressure
{
public:
    virtual pressure getPressure() = 0;
    virtual depth getDepth() = 0;

private:
};

#endif