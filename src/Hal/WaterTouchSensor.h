#ifndef WATERTOUCHSENSOR_H
#define WATERTOUCHSENSOR_H

#include <Arduino.h>
#include <Preferences.h>

class WaterTouchSensor
{
public:
    WaterTouchSensor(uint8_t pin, uint16_t threshold);
    void begin();
    bool isWaterDetected();

    void setThreshold(uint16_t threshold);
    uint16_t getThreshold() const;

    void updateAirValue();
    void updateWaterValue();
    float getLowLimit() const;
    float getHighLimit() const;
    void updateThreshold();

    void loadFromMemory();
    void saveToMemory();

    void logLimits();

private:
    uint8_t _pin;
    uint16_t _threshold;

    static const uint8_t SAMPLE_COUNT = 5;
    uint16_t _airValues[SAMPLE_COUNT];
    uint16_t _waterValues[SAMPLE_COUNT];
    uint8_t _airIndex;
    uint8_t _waterIndex;

    Preferences _preferences;

    float calculateAverage(const uint16_t values[], uint8_t count) const;
    bool isValueValid(uint16_t value) const;
    uint16_t read();
};

#endif // WATERTOUCHSENSOR_H
