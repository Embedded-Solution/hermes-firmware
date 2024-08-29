#ifndef GNSS_HPP
#define GNSS_HPP

#include <Arduino.h>
#include <Hal/TinyGPS++.h>
#include <TimeLib.h>

#include <Navigation/Navigation.hpp>
#include <Hal/remora-hal.h>
#include <Types.hpp>

#include <Wire.h>
#include <hal/TSYS01.hpp>
#include <hal/MS5837.hpp>
#include <Settings.hpp>
#include <Dive.hpp>

struct Position
{
    lat Lat;
    lng Lng;
    time_t dateTime;
    bool valid;
};

class GNSS : public Navigation
{
public:
    GNSS();

    lat getLat();
    lng getLng();
    Position parseStart(struct Record *records);
    void parse();
    Position parseEnd(struct Record *records, int recordsLength);
    time_t getTime();

private:
    HardwareSerial GPSSerial = Serial2;
    TinyGPSPlus gps;
};

#endif