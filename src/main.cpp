#include <Arduino.h>
#include <Wake.hpp>
#include "Settings.hpp"

TaskHandle_t Task1;

void setup()
{
    Serial.begin(115200);
    delay(100);
 Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());
    


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