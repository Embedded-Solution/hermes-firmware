#include <Wake.hpp>

using namespace std;
SecureDigital sd;
WifiManager wm;
WaterTouchSensor waterSensor(WATER_TOUCH_PIN, END_DIVE_WATER_THRESHOLD);

// variables permanentes pour le mode de plongée statique
RTC_DATA_ATTR Dive staticDive(&sd);
RTC_DATA_ATTR bool diveMode = 0; // 0:dynamic, 1:static
RTC_DATA_ATTR int staticCount;
RTC_DATA_ATTR long staticTime;

/// @brief Interrupt routine to shutdown remora if wifi is disconnected
/// @return
void IRAM_ATTR ISR()
{
    sleep(DEFAULT_SLEEP);
}

void wake()
{
    waterSensor.begin();

    // setup gpios
    log_i("firmware version:%s\n", FIRMWARE_VERSION);
    sd.writeFile("/version.txt", String(FIRMWARE_VERSION));

    pinMode(GPIO_LED1, OUTPUT);
    pinMode(GPIO_LED2, OUTPUT);
    pinMode(GPIO_LED3, OUTPUT);
    pinMode(GPIO_LED4, OUTPUT);
    pinMode(GPIO_VBATT, INPUT);
    digitalWrite(GPIO_LED3, LOW);
    digitalWrite(GPIO_LED4, LOW);

    pinMode(GPIO_PROBE, OUTPUT); // set gpio probe pin as low output to avoid corrosion
    digitalWrite(GPIO_PROBE, LOW);
    pinMode(GPIO_WATER, OUTPUT);
    digitalWrite(GPIO_WATER, LOW);

    // check if sd card is ready, if not go back to sleep without water detection wake up
    if (sd.ready() == false)
        sleep(SDCARD_ERROR_SLEEP);

    // check wake up reason
    uint64_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
    {
        log_d("Wake up timer static");
        gpio_hold_dis(GPIO_NUM_33);
        staticDiveWakeUp();
    }
    else
    {
        wakeup_reason = esp_sleep_get_ext1_wakeup_status();
        uint64_t mask = 1;
        int i = 0;

        // if wake up with water sensor, start dive
        if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD)
        {
            dynamicDive();
        }

        while (i < 64)
        {
            if (wakeup_reason & mask)
            {
                if (i == GPIO_WATER) // Start dive
                {
                    if (diveMode == STATIC_MODE)
                    { // if Water wake up and static Mode
                        log_d("Static dive");
                        startStaticDive();
                        sleep(SLEEP_WITH_TIMER);
                    }
                    else
                    {
                        log_d("Dynamic dive");
                        dynamicDive();
                    }
                }
                else if (i == GPIO_VCC_SENSE) // wifi config
                {
                    log_i("Wake up gpio vcc sense");

                    log_i("Start Check Index ");
                    Dive d(&sd);
                    d.checkIndex();
                    log_d("End Check Index ");

                    // While wifi not set, shutdown if usb is disconnected
                    attachInterrupt(GPIO_VCC_SENSE, ISR, FALLING);

                    wm.startPortal(sd);
                }
                else if (i == GPIO_CONFIG) // button config (switch between diving modes)
                {
                    log_d("Wake up gpio config, check delete credentials");
                    if (wm.checkDeleteCredentials() == false)
                    {
#ifdef MODE_DEBUG
                        dynamicDive();
#else
                        selectMode();
#endif
                    }
                }
            }

            i++;
            mask = mask << 1;
        }
    }
}

void dynamicDive()
{

    pinMode(GPIO_PROBE, OUTPUT); // set gpio probe pin as low output to avoid corrosion
    digitalWrite(GPIO_PROBE, LOW);
    pinMode(GPIO_WATER, OUTPUT);
    digitalWrite(GPIO_WATER, LOW);

    // detect if the wake up is because of diving or not
    // If not, do not start dynamic dive
// detect if the wake up is because of diving or not
// If not, do not start recording
#ifdef MODE_DEBUG
    if (true)
#else
    if (detectSurface(BEGIN_SURFACE_DETECTION))
#endif
    {
        // set gpio probe pin as low output to avoid corrosion
        pinMode(GPIO_PROBE, OUTPUT);
        digitalWrite(GPIO_PROBE, LOW);

        // set gpio sensor power pin as low output to power sensors
        pinMode(GPIO_SENSOR_POWER, OUTPUT);
        digitalWrite(GPIO_SENSOR_POWER, LOW);

        delay(10);
        Wire.begin(I2C_SDA, I2C_SCL);
        delay(10);

        // Init sensors
        GNSS gps = GNSS();
        sd = SecureDigital();
        Dive d(&sd);
        tsys01 temperatureSensor = tsys01();
        ms5837 depthSensor = ms5837();

        bool led_on = false;
        bool endDive = false;

        // Init struct for recording during gps research
        int len = TIME_GPS_START / (TIME_GPS_RECORDS);
        struct Record gpsRecords[len + 1];
        for (int x = 0; x < len; x++)
            gpsRecords[x] = {-1000, 0, 0};

        // get gps position, dateTime and records during gps search.
        Position startPos = gps.parseStart(gpsRecords);

        if (d.Start(startPos.dateTime, startPos.Lat, startPos.Lng, TIME_DYNAMIC_MODE, diveMode) != "")
        {
            int timer = 0;

            // save records from gps search
            for (int i = 0; i < len; i++)
            {
                if (gpsRecords[i].Temp > -1000)
                {
                    timer += TIME_GPS_RECORDS;
                    d.NewRecord(gpsRecords[i]);
                }
            }

            bool validDive = false;
            int count = 0;
            double depth, temp;
            unsigned long previousTime = 0, currentTime = 0;
            int secondCount = 0;
            bool lowBat = false, vccSense = false;

            // if valid dive, dive end after short time, if dive still not valid, dive end after long time
            while (!endDive)
            {
                currentTime = gps.getTime();

                if (currentTime != previousTime) // check if time changed
                {
                    secondCount++;
                    previousTime = currentTime; // reset previous time
                }

                if (secondCount >= TIME_DYNAMIC_MODE) // if new records required
                {
                    secondCount = 0;
                    timer += TIME_DYNAMIC_MODE; // get time in seconds since wake up

                    temp = temperatureSensor.getTemp();
                    depth = depthSensor.getDepth();
                    log_v("Temp = %2.2f\t Depth = %3.3f\t Pressure = %4.4f", temp, depth, depthSensor.getPressure());

                    ///////////////// Detect end of dive ////////////////////
                    // if dive still not valid, check if depthMin reached
                    if (validDive == false)
                    {
                        if (depth > MIN_DEPTH_VALID_DIVE)
                        {
                            log_d("Valid Dive, reset counter end dive");
                            validDive = true; // if minDepth reached, dive is valid
                            count = 0;        // reset count before detect end of dive
                        }
                    }

                    // Save record
                    Record tempRecord = Record{temp, depth, timer};
                    d.NewRecord(tempRecord);

                    // blink led
                    if (led_on)
                        digitalWrite(GPIO_LED4, HIGH);
                    else
                        digitalWrite(GPIO_LED4, LOW);
                    led_on = !led_on;

                    // check battery, back to sleep  without water detection if lowBat
                    if (timer % TIME_CHECK_POWER == 0)
                    {
                        if (readBattery() < LOW_BATTERY_LEVEL)
                        {
                            lowBat = true;
                            endDive = true;
                            log_d("LOW BATT DETECTED");
                        }

                        pinMode(GPIO_VCC_SENSE, INPUT);
                        if (digitalRead(GPIO_VCC_SENSE))
                        {
                            endDive = true;
                            vccSense = true;
                            log_d("GPIO VCC SENSE DETECTED");
                        }
                        pinMode(GPIO_VCC_SENSE, OUTPUT);
                    }

                    // if depth is low (near surface), check water sensor to detect end of dive.
                    if (depthSensor.getDepth() < MIN_DEPTH_CHECK_END_DIVE)
                    {
                        // TODO check water sensor to detect end of dive.
                        if (waterSensor.isWaterDetected() == false)
                            count++;
                        else
                            count = 0;
                        log_v("Water : %d\tAverage : %3.3f\tCount : %d", checkWater(), depthSensor.getDepth(), count);
                    }
                    else
                    {
                        count = 0;
                    }

                    if (count >= (validDive == true ? MAX_DYNAMIC_COUNTER_VALID_DIVE : MAX_DYNAMIC_COUNTER_NO_DIVE))
                    {
                        endDive = true;
                    }
                    /////////////////// Depth Running Amplitude & average to detect end of dive/////////////
                }
            }

            log_d("valid dive = %d\tvccsense = %d\t valid pos = %d\t lowbatt = %d", validDive, vccSense, startPos.valid, lowBat);
            String end;
            // if dive valid (Pmin reached) get end GPS, else delete records and clean index
            if (validDive == true && lowBat == false && vccSense == false)
            {
                log_d("Normal end dive");
                // Init struct for recording during gps research
                int len = TIME_END_RECORDS / (TIME_GPS_RECORDS);
                struct Record gpsRecords[len];
                for (int x = 0; x < len; x++)
                    gpsRecords[x] = {-1000, 0, 0};

                // get gps position, dateTime and records during gps search.
                Position endPos = gps.parseEnd(gpsRecords, len, timer);

                // save records from gps search
                for (int i = 0; i < len; i++)
                {
                    if (gpsRecords[i].Temp > -1000)
                    {
                        timer += TIME_GPS_RECORDS;
                        d.NewRecord(gpsRecords[i]);
                    }
                }

                end = d.End(endPos.dateTime, endPos.Lat, endPos.Lng, diveMode);

                if (!startPos.valid && !endPos.valid)
                {
                    log_d("Dive valid but no valid pos, record deleted");
                    d.deleteID(d.getID());
                }
            }
            else if (validDive == true && lowBat == true && startPos.valid == true) // if lowbat and  datetime and gps ok save datas and set ready to upload
            {
                end = d.End(gps.getTime(), 0, 0, diveMode);
                log_d("End dive after low batt");
                sleep(LOW_BATT_SLEEP);
            }
            else if (validDive == true && vccSense == true && startPos.valid == true) // if usb connected datetime and gps ok , end dive only with timer
            {
                log_d("End dive after VCC SENSE");
                end = d.End(gps.getTime(), 0, 0, diveMode);
            }
            else
            {
                log_d("Dive not valid, record deleted");
                d.deleteID(d.getID());

                if (lowBat == true) // if dive not valid AND lowBatt, low batt sleep
                    sleep(LOW_BATT_SLEEP);
            }

            if (end == "")
            {
                log_e("error ending the dive");
            }
        }
    }
    else
    {
        log_d("Surface not detected");
    }
}

void startStaticDive()
{
    pinMode(GPIO_SENSOR_POWER, OUTPUT);
    digitalWrite(GPIO_SENSOR_POWER, LOW);
    delay(10);
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(10);

    GNSS gps = GNSS();
    sd = SecureDigital();
    tsys01 temperatureSensor = tsys01();
    ms5837 depthSensor = ms5837();

    staticCount = 0;

    if (staticDive.Start(now(), gps.getLat(), gps.getLng(), TIME_TO_SLEEP_STATIC, diveMode) != "")
    {
        pinMode(GPIO_LED4, OUTPUT);
        for (int i = 0; i < 3; i++)
        {
            digitalWrite(GPIO_LED4, HIGH);
            delay(300);
            digitalWrite(GPIO_LED4, LOW);
            delay(300);
        }
        double depth, temp;
        staticTime = 0;

        temp = temperatureSensor.getTemp();
        depth = depthSensor.getDepth();

        Record tempRecord = Record{temp, depth, staticTime};
        staticDive.NewRecordStatic(tempRecord);
    }
}

void staticDiveWakeUp()
{
    pinMode(GPIO_PROBE, OUTPUT); // set gpio probe pin as low output to avoid corrosion
    digitalWrite(GPIO_PROBE, LOW);
    pinMode(GPIO_SENSOR_POWER, OUTPUT);
    digitalWrite(GPIO_SENSOR_POWER, LOW);
    delay(10);
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(10);

    tsys01 temperatureSensor = tsys01();
    ms5837 depthSensor = ms5837();
    double depth, temp;

    temp = temperatureSensor.getTemp();
    depth = depthSensor.getDepth();

    staticTime += TIME_TO_SLEEP_STATIC;

    if (depth < MAX_DEPTH_CHECK_WATER)
    {
        pinMode(GPIO_PROBE, INPUT); // enable probe pin to allow water detection
        pinMode(GPIO_WATER, INPUT);
        int value;

        value = analogRead(GPIO_WATER);
        log_v("Value = %d", value);

        if (value >= WATER_TRIGGER)
            staticCount = 0; // reset No water counter
        else
            staticCount++;           // if no water counter++
        pinMode(GPIO_PROBE, OUTPUT); // set gpio probe pin as low output to avoid corrosion
        digitalWrite(GPIO_PROBE, LOW);
        pinMode(GPIO_WATER, OUTPUT);
        digitalWrite(GPIO_WATER, LOW);
    }

    Record tempRecord = Record{temp, depth, staticTime};
    staticDive.NewRecordStatic(tempRecord);

    // check battery and USB power , back to sleep  without water detection if lowBat
    if (staticTime % TIME_CHECK_POWER == 0)
        if (readBattery() < LOW_BATTERY_LEVEL)
            sleep(LOW_BATT_SLEEP);

    if (staticCount < MAX_STATIC_COUNTER) // if water detected sleep with timer
        sleep(SLEEP_WITH_TIMER);
    else // if no water, end static dive
    {
        GNSS gps = GNSS();
        String ID = staticDive.End(now(), gps.getLat(), gps.getLng(), diveMode);
        if (ID == "")
            log_e("error ending the dive");
        sleep(DEFAULT_SLEEP); // sleep without timer waiting for other dive or config button
    }
}

void selectMode()
{
    diveMode = !diveMode;

    if (diveMode == STATIC_MODE)
    {
        log_v("Static Diving");

        digitalWrite(GPIO_LED4, HIGH);
        delay(3000);
        digitalWrite(GPIO_LED4, LOW);
    }
    else
    {
        log_v("Dynamic diving");

        for (int i = 0; i < 10; i++)
        {
            digitalWrite(GPIO_LED4, HIGH);
            delay(150);
            digitalWrite(GPIO_LED4, LOW);
            delay(150);
        }
    }
}

// 1 : underwater, 0 : air
bool detectSurface(float levelSurfaceDetection)
{
    pinMode(GPIO_SENSOR_POWER, OUTPUT);
    digitalWrite(GPIO_SENSOR_POWER, LOW);
    delay(10);
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(10);

    ms5837 depthSensor = ms5837();
    double depth = 0, min = 999, max = -999;
    int count = 0, avgCount = 0;
    double avg = 0;
    unsigned long start = millis();
    while (millis() - start <= TIME_SURFACE_DETECTION * 1000)
    {

        while (count < 10)
        {
            count++;
            depth = depthSensor.getDepth();
            log_v("Depth = %f", depth);
            if (depth < min)
                min = depth;
            if (depth > max)
                max = depth;
            delay(50);
        }
        avg += max - min;
        max = -999;
        min = 999;
        avgCount++;
        count = 0;
    }

    if (avg / (float)avgCount > levelSurfaceDetection)
        return 1;
    else
        return 0;
}