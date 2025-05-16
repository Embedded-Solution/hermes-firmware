#ifndef WAKE_HPP
#define WAKE_HPP
#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <AutoConnect.h>
#include <Connect/WifiManager.hpp>
#include <Dive.hpp>
#include <hal/TSYS01.hpp>
#include <hal/MS5837.hpp>
#include <hal/remora-hal.h>
#include <hal/WaterTouchSensor.h>
#include <Storage/SecureDigital.hpp>
#include <Navigation/GNSS.hpp>
#include <Utils.hpp>
#include <Connect/Connect.hpp>
#include <Settings.hpp>
#include <Hal/WaterTouchSensor.h>

void wake();
void dynamicDive();
bool detectSurface(float levelSurfaceDetection);

#endif