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

//FRM DEFINES POUR TS
#define TOUCH_THRESH_NO_USE   (0)
#define TOUCH_THRESH_PERCENT  (80)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)
#define TSSEUIL 300

#endif