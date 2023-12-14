#include <Utils.hpp>

String remoraID()
{
    wakeModemSleep();

    byte shaResult[32];
    WiFi.mode(WIFI_MODE_STA);
    String unhashed_id = WiFi.macAddress();
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    const size_t payloadLength = strlen(unhashed_id.c_str());

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char *)unhashed_id.c_str(), payloadLength);
    mbedtls_md_finish(&ctx, shaResult);
    mbedtls_md_free(&ctx);

    String hash;
    for (int i = 0; i < sizeof(shaResult); i++)
    {
        char str[3];

        sprintf(str, "%02x", (int)shaResult[i]);
        hash = hash + str;
    }
    setModemSleep();

    log_v("remoraID = %s", hash.c_str());
    return hash;
}

void sleep(int mode)
{
    SecureDigital sd;

    String path = "/sleep.txt";
    if (sd.findFile(path) == -1)
        sd.writeFile(path, String(mode));
    else
        sd.appendFile(path, String(mode));

    uint64_t wakeMask;
    switch (mode)
    {
        // if other mode, wake up with water, config, or charging
    case DEFAULT_SLEEP:
        log_v("DEFAULT SLEEP");
        pinMode(GPIO_PROBE, INPUT); // Set GPIO PROBE back to input to allow water detection
#ifndef MODE_DEBUG
        wakeMask = 1ULL << GPIO_WATER | 1ULL << GPIO_CONFIG | 1ULL << GPIO_VCC_SENSE;
#else
        wakeMask = 1ULL << GPIO_WATER | 1ULL << GPIO_CONFIG;
#endif
        esp_sleep_enable_ext1_wakeup(wakeMask, ESP_EXT1_WAKEUP_ANY_HIGH);

        break;

        // if static diving, wake up with timer or config button
    case SLEEP_WITH_TIMER:
        log_v("SLEEP WITH TIMER");
        pinMode(GPIO_PROBE, OUTPUT); // set gpio probe pin as low output to avoid corrosion
        digitalWrite(GPIO_PROBE, LOW);
        gpio_hold_en(GPIO_NUM_33);
        gpio_deep_sleep_hold_en();

        wakeMask = 1ULL << GPIO_CONFIG;
        esp_sleep_enable_ext1_wakeup(wakeMask, ESP_EXT1_WAKEUP_ANY_HIGH);
        esp_sleep_enable_timer_wakeup((TIME_TO_SLEEP_STATIC * 1000 - OFFSET_SLEEP_STATIC) * 1000);
        break;

        // if sd card error sleep without water detection
    case SDCARD_ERROR_SLEEP:
    case LOW_BATT_SLEEP:
        log_v("SD CARD ERROR OR LOW BATT SLEEP");
        pinMode(GPIO_PROBE, OUTPUT); // set gpio probe pin as low output to avoid corrosion
        digitalWrite(GPIO_PROBE, LOW);
        gpio_hold_en(GPIO_NUM_33);
        gpio_deep_sleep_hold_en();
#ifndef MODE_DEBUG
        wakeMask = 1ULL << GPIO_CONFIG | 1ULL << GPIO_VCC_SENSE;
#else
        wakeMask = 1ULL << GPIO_CONFIG;
#endif
        esp_sleep_enable_ext1_wakeup(wakeMask, ESP_EXT1_WAKEUP_ANY_HIGH);
        break;
    }

    log_i("Going to sleep now");
    esp_deep_sleep_start();
}

float readBattery()
{
    pinMode(GPIO_VBATT, INPUT_PULLUP);
    float val = analogRead(GPIO_VBATT);
    float vBat = ((val / 4095.0)) * 3.3 * 1.33;
    log_i("Batterie = %1.2f", vBat);
    pinMode(GPIO_VBATT, OUTPUT);

    return vBat;
}

void TaskLedBatteryCode(void *parameter)
{
    unsigned long previousMillisLed = 0, previousMillisBattery = 0;
    bool ledState = HIGH;
    float batteryLevel = readBattery();
    for (;;)
    {
        unsigned long currentMillis = millis();

        // check batteryLevel every 60s
        if (currentMillis - previousMillisBattery >= 60000)
        {
            batteryLevel = readBattery();
            previousMillisBattery = currentMillis;
        }

        if (batteryLevel < BATTERY_LEVEL_25)
        {
            digitalWrite(GPIO_LED4, LOW); // led diving off
            if (currentMillis - previousMillisLed >= 500)
            {
                ledState = (ledState == LOW) ? HIGH : LOW;
                digitalWrite(GPIO_LED1, ledState);
                previousMillisLed = currentMillis;
            }
        }

        else if (batteryLevel > BATTERY_LEVEL_75)
        {
            digitalWrite(GPIO_LED1, LOW);  // led error off
            digitalWrite(GPIO_LED4, HIGH); // led diving on
        }
        else
        {
            // all leds off
            digitalWrite(GPIO_LED1, LOW);
            digitalWrite(GPIO_LED4, LOW);
        }
    }
}


// Method to compare two versions. 
// Returns 1 if v2 is smaller, -1 
// if v1 is smaller, 0 if equal 
int versionCompare(string v1, string v2) 
{ 
    // vnum stores each numeric 
    // part of version 
    int vnum1 = 0, vnum2 = 0; 
 
    // loop until both string are 
    // processed 
    for (int i = 0, j = 0; (i < v1.length() 
                            || j < v2.length());) { 
        // storing numeric part of 
        // version 1 in vnum1 
        while (i < v1.length() && v1[i] != '.') { 
            vnum1 = vnum1 * 10 + (v1[i] - '0'); 
            i++; 
        } 
 
        // storing numeric part of 
        // version 2 in vnum2 
        while (j < v2.length() && v2[j] != '.') { 
            vnum2 = vnum2 * 10 + (v2[j] - '0'); 
            j++; 
        } 
 
        if (vnum1 > vnum2) 
            return 1; 
        if (vnum2 > vnum1) 
            return -1; 
 
        // if equal, reset variables and 
        // go for next numeric part 
        vnum1 = vnum2 = 0; 
        i++; 
        j++; 
    } 
    return 0; 
}

void setModemSleep(void)
{
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    WiFi.setSleep(true);
    if (!setCpuFrequencyMhz(40))
    {
        Serial2.println("Not valid frequency!");
    }

    // Use this if 40Mhz is not supported
    // setCpuFrequencyMhz(80);
}

void wakeModemSleep(void)
{
    setCpuFrequencyMhz(240);
    delay(1);
}