#include <Arduino.h>
#include <Wake.hpp>
#include "Settings.hpp"

TaskHandle_t Task1;

File logFile;

int sdCardLogOutput(const char *format, va_list args)
{
    Serial.println("Callback running");
    char buf[128];
    int ret = vsnprintf(buf, sizeof(buf), format, args);
    if (logFile)
    {
        logFile.print(buf);
        logFile.flush();
    }
    return ret;
}

void setup()
{
    Serial.begin(115200);
    delay(100);

#ifdef LOG_ON_SD_CARD
    if (SD.begin())
    {
        logFile = SD.open("/log.txt", FILE_APPEND);
        if (!logFile)
        {
            Serial.println("Failed to open log file");
        }
        else
        {
            Serial.println("Setting log levels and callback");
            esp_log_level_set(TAG, LOG_LEVEL);
            esp_log_set_vprintf(sdCardLogOutput);
        }
    }
#endif

#ifdef SERIAL1_DEBUG_OUTPUT
    Serial1.begin(115200);
    delay(100);
    Serial1.setDebugOutput(true);
#endif

    wake();
    sleep(DEFAULT_SLEEP);
}

void loop()
{
}