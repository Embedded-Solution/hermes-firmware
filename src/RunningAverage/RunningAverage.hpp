#ifndef RunningAverage_hpp
#define RunningAverage_hpp
//
//    FILE: RunningAverage.h
//  AUTHOR: Rob dot Tillaart at gmail dot com
// PURPOSE: RunningAverage library for Arduino
//     URL: https://arduino.cc/playground/Main/RunningAverage
// HISTORY: See RunningAverage.cpp
//
// Released to the public domain
//

// backwards compatibility
// clr() clear()
// add(x) addValue(x)
// avg() getAverage()

#define RUNNINGAVERAGE_LIB_VERSION "0.2.04"

#include "Arduino.h"

class RunningAverage
{
public:
    RunningAverage(void);
    RunningAverage(int);
    ~RunningAverage();

    void clear();
    void addValue(float);
    void fillValue(float, int);

    float getAverage();
    float getAmplitude();

    float getElement(uint8_t idx);
    uint8_t getSize() { return _size; }
    uint8_t getCount() { return _cnt; }
    float getMin() { return _min; }
    float getMax() { return _max; }

protected:
    uint8_t _size;
    uint8_t _cnt;
    uint8_t _idx;
    float _sum;
    float *_ar;
    float _min;
    float _max;
};

#endif