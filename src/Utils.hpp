#ifndef UTILS_HPP
#define UTILS_HPP

#include <WiFi.h>
#include <mbedtls/md.h>
#include <Settings.hpp>
#include <hal/remora-hal.h>
#include <Storage/SecureDigital.hpp>

String remoraID();
void sleep(int mode = 0);
float readBattery();
void TaskLedBatteryCode(void *parameter);
int versionCompare(string v1, string v2);

#endif