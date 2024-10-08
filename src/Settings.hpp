#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#define DEFAULT_SLEEP 0
#define SLEEP_WITH_TIMER 1
#define SDCARD_ERROR_SLEEP 2
#define LOW_BATT_SLEEP 3

#define FIRMWARE_VERSION "4.05"
// Debug parameters
#ifdef TAG
#undef TAG
#define TAG (char *)"REMORA"
#endif
// #define MODE_DEBUG
// #define SERIAL1_DEBUG_OUTPUT
#define LOG_ON_SD_CARD
#define LOG_LEVEL ESP_LOG_DEBUG

// Dives parameters
#define OFFSET_SLEEP_STATIC 1700         /*Offset going to sleep and wake up static mode*/
#define TIME_TO_SLEEP_STATIC 10          /* Time ESP32 will go to sleep (in seconds) between each static record */
#define TIME_DYNAMIC_MODE 1              /* Time between two records in dynamic mode (s)*/
#define TIME_GPS_RECORDS 5               /* Time between records during gps search*/
#define MIN_DEPTH_VALID_DIVE 2           /* Min depth to validate dynamic dive (meters)*/
#define MAX_DEPTH_CHECK_WATER 0.5        /* Max depth for water detection (meter)*/
#define MAX_DEPTH_CHECK_GPS 1.0          /* Max depth for gps detection (meter)*/
#define MAX_DYNAMIC_COUNTER_VALID_DIVE 1 /* Number of No Water to end dynamic dive (TIME_DYNAMIC_MODE  * MAX_DYNAMIC_COUNTER_VALID_DIVE = seconds)*/
#define MAX_DYNAMIC_COUNTER_NO_DIVE 300  /* Number of No Water to end dynamic dive (TIME_DYNAMIC_MODE  * MAX_DYNAMIC_COUNTER_NO_DIVE = seconds) at the beginning of the dive*/
#define MAX_STATIC_COUNTER 2             /* Number of No Water to end static dive (TIME_TO_SLEEP_STATIC * MAX_STATIC_COUNTER = seconds )*/
#define WATER_TRIGGER 1500               // mV water detection level (0 to 3300) (lower value for pure water)
#define TIME_GPS_START 600               // research time gps at the beginning of the dive(seconds)
#define TIME_GPS_END 900                 // research time gps at the end of the dive (seconds)
#define TIME_END_RECORDS 180             // time recording depth/temp at the end of the dive
#define TIME_SURFACE_DETECTION 5         // Time to detect surface crossing at the beginning of the dive (seconds)
#define BEGIN_SURFACE_DETECTION 0.05     // depth max-min to detect surface crossing at the beggining of the dive
#define LOW_BATTERY_LEVEL 3.3            //  If vBat < Low battery level, go back to sleep without water detection wakeup (Volts)
#define BATTERY_LEVEL_25 3.7             //  If vBat < Low battery level, go back to sleep without water detection wakeup (Volts)
#define BATTERY_LEVEL_75 4.2               //  If vBat < Low battery level, go back to sleep without water detection wakeup (Volts)
#define TIME_UPLOAD_OTA 3600             // Time between 2 upload and OTA check (seconds)
#define TIME_CHECK_POWER 60              // Time between 2 battery ccheck during dynamic dive. (seconds)
#define MIN_DEPTH_CHECK_END_DIVE 3.0     // min depth to check water sensor before ending dive. (meter)
#define ENDING_DIVE_DEPTH_AMPLITUDE 0.15 // min depth amplitude to end dive, if depth amplitude below val, diver is out of water.
#define SAMPLES_NUMBER_DEPTH_CHECK 30    // number of samples to calculate average depth and depth amplitude during end of dive
#define TIMER_DEL_CREDENTIALS 10         // Time to press config button to delete all credentials
#define TIME_CHECK_BATTERY 60000         // Time to check battery level while charging

// Water sensor parameters
#define WATER_TOUCH_PIN TOUCH_PAD_NUM2
#define WAKE_UP_WATER_THRESHOLD 350
#define END_DIVE_WATER_THRESHOLD 400

#define TOUCH_THRESH_NO_USE (0)
#define TOUCH_THRESH_PERCENT (80)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)

// Upload parameters
const String indexPath = "/index.json";
#define POST_RETRY 3 // number of post attemp before skip.

#endif
