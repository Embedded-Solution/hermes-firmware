#ifndef ms5837_h
#define ms5837_h

#include <Arduino.h>

#include <Sensors/Pressure.hpp>
#include <Sensors/Depth.hpp>
#include <Sensors/Temperature.hpp>
#include <Types.hpp>

class ms5837: public Pressure, public Depth, public Temperature
{
  public:
    ms5837();
    temperature getTemp();
    pressure getPressure();
    depth getDepth();

  private:
    const short _addr = 0x76;
    const short _reset = 0x1E;
    const short _adc_read = 0x00;
    const short _prom_read = 0xA0;
    const short _d1_8192 = 0x4A;
    const short _d2_8192 = 0x5A;

    uint64_t _rawTemp;
    uint64_t _rawPres;

    int64_t _deltaTemp;
    int64_t _temp;
    int64_t _pressure;

    const int64_t _fluidDensity = 1029;
    uint64_t _msCalibrationValue[8];

    const double _pa = 100.0f;

    uint8_t crc4(uint64_t[]);
    void readValues();
    void computeTemp();
    void computePressure();
};

#endif