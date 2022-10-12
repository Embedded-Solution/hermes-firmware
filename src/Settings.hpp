#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#define FIRMWARE_VERSION 2.6

// Dives parameters
#define uS_TO_S_FACTOR 1000000    /* Conversion factor for micro seconds to seconds */
#define OFFSET_SLEEP_STATIC 1750    /*Offset going to sleep and wake up static mode*/
#define TIME_TO_SLEEP_STATIC 5    /* Time ESP32 will go to sleep (in seconds) between each static record */
#define TIME_DYNAMIC_MODE 1000    /* Time between two records in dynamic mode (ms)*/
#define MIN_DEPTH_VALID_DIVE -9999/* Min depth to validate dynamic dive (meters)*/
#define MAX_DEPTH_CHECK_WATER 0.5 /* Max depth for water detection (meter)*/
#define MAX_DYNAMIC_COUNTER 20    /* Number of No Water to end dynamic dive (TIME_DYNAMIC_MODE / 1000 * MAX_DYNAMIC_COUNTER = seconds)*/
#define MAX_STATIC_COUNTER 2      /* Number of No Water to end static dive (TIME_TO_SLEEP_STATIC * MAX_STATIC_COUNTER = seconds )*/
#define WATER_TRIGGER 1500        // mV water detection level (0 to 3300) (lower value for pure water)
#define TIME_GPS 300              // research time gps (seconds)

// Upload parameters
const String indexPath = "/index.json";
const String metadataURL = "https://project-hermes.azurewebsites.net/api/Remora";
const String recordURL = "https://project-hermes.azurewebsites.net/api/RemoraRecord";
const String firmwareURL = "https://project-hermes.azurewebsites.net/api/Firmware";

#endif
