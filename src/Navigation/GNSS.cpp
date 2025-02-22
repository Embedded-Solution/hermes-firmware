#include <Navigation/GNSS.hpp>
#include "GNSS.hpp"

GNSS::GNSS()
{
    // parse();
}

lat GNSS::getLat()
{
    // parse();
    return (lat)gps.location.lat();
}

lng GNSS::getLng()
{
    // parse();
    return (lng)gps.location.lng();
}

time_t GNSS::getTime()
{
    struct tm tm;  // tm struct to store date time from gps
    char nextData; // char received from gps
    bool timeReady = false;

    pinMode(GPIO_GPS_POWER, OUTPUT);
    digitalWrite(GPIO_GPS_POWER, LOW);

    while (timeReady == false)
    {
        while (GPSSerial.available())
        {
            nextData = GPSSerial.read();
            // Serial1.write(nextData);
            if (gps.encode(nextData))
            {
                // Serial1.write("\n");

                if (gps.time.isUpdated() && gps.date.isUpdated())
                {
                    tm.tm_hour = gps.time.hour();
                    tm.tm_min = gps.time.minute();
                    tm.tm_sec = gps.time.second();
                    tm.tm_year = gps.date.year() - 1900;
                    tm.tm_mon = gps.date.month() - 1;
                    tm.tm_mday = gps.date.day();

                    timeReady = true;
                }
            }
        }
    }

    log_v("Date: %d-%d-%d\t Time : %d:%d:%d", gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());

    return mktime(&tm); // return time and date from gps converted into timestamp
}

Position GNSS::parseStart(struct Record *records)
{
    Position pos = {0};
    digitalWrite(GPIO_LED2, HIGH);
    pinMode(GPIO_GPS_POWER, OUTPUT);
    digitalWrite(GPIO_GPS_POWER, LOW);

    GPSSerial.begin(9600);
    delay(500); // TODO this needs to be more dynamic
    unsigned long start = millis();
    bool gpsOK = false, timeOK = false;

    pinMode(GPIO_SENSOR_POWER, OUTPUT);
    digitalWrite(GPIO_SENSOR_POWER, LOW);
    delay(10);
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(10);

    tsys01 temperatureSensor = tsys01();
    ms5837 depthSensor = ms5837();
    double temp = temperatureSensor.getTemp();
    double depth = depthSensor.getDepth();

    unsigned long previousTime = 0, currentTime = 0;
    int idRecord = 0;
    int count = 0;

    currentTime = getTime(); // Init Current TIme

    while (millis() < start + TIME_GPS_START * 1000 && (!gpsOK || !timeOK) && depth < MAX_DEPTH_CHECK_GPS)
    {
        if (GPSSerial.available() > 0 && gps.encode(GPSSerial.read()))
        {
            if (gps.date.isValid() && gps.time.isValid())
            {
                log_v("Date: %d/%d/%d\tHour: %d:%d:%d",
                      gps.date.day(), gps.date.month(), gps.date.year(),
                      gps.time.hour(), gps.time.minute(), gps.time.second());

                TimeElements gpsTime = {
                    (uint8_t)gps.time.second(),
                    (uint8_t)gps.time.minute(),
                    (uint8_t)gps.time.hour(),
                    0,
                    (uint8_t)gps.date.day(),
                    (uint8_t)gps.date.month(),
                    (uint8_t)(gps.date.year() - 1970)};
                setTime(makeTime(gpsTime));

                if (year() > 2020 && year() < 2050 && !timeOK)
                {
                    timeOK = true;
                    pos.dateTime = now();
                    log_d("DateTime: %ld\tNow:%ld", pos.dateTime, now());
                }
            }
            if (gps.location.isValid())
            {
                log_d("Position: %f , %f", getLat(), getLng());
                gpsOK = true;
                pos.Lat = (lat)gps.location.lat();
                pos.Lng = (lng)gps.location.lng();
            }
            depth = depthSensor.getDepth();
            currentTime = getTime();

            if (currentTime != previousTime) // check if time changed
            {
                count++;
                previousTime = currentTime; // reset previous time
            }

            if (count >= TIME_GPS_RECORDS && idRecord < sizeof(records)) // if new records required and array not full
            {
                count = 0;

                // save temp and depth
                temp = temperatureSensor.getTemp();
                records[idRecord].Depth = depth;
                records[idRecord].Temp = temp;
                records[idRecord].Time = (idRecord + 1) * TIME_GPS_RECORDS;
                idRecord++;
                log_v("ID records during gps search : %d", idRecord);
            }
        }
    }
    digitalWrite(GPIO_LED2, LOW);                       // turn syn led off when gps connected
    pos.dateTime = now() - idRecord * TIME_GPS_RECORDS; // start datetime is before the gps search so we remove the duration of the gps search.
    log_v("DateTime: %ld\tNow:%ld", pos.dateTime, now());

    if (timeOK && gpsOK) // save if datetime and position is ok
    {
        pos.valid = true;
        log_d("POsition and dateTime valid");
    }

    return pos;
}

Position GNSS::parseEnd(struct Record *records, int recordsLength, int oldTime)
{
    Position pos = {0};
    digitalWrite(GPIO_LED2, HIGH);
    pinMode(GPIO_GPS_POWER, OUTPUT);
    digitalWrite(GPIO_GPS_POWER, LOW);

    GPSSerial.begin(9600);
    delay(500); // TODO this needs to be more dynamic
    unsigned long start = millis();
    bool gpsOK = false, timeOK = false, recordsOK = false;
    bool fixEphemeridesOK = true; // true au démarrage pour ne pas bloquer si aucun fix gps n'alieu dans le délai

    pinMode(GPIO_SENSOR_POWER, OUTPUT);
    digitalWrite(GPIO_SENSOR_POWER, LOW);
    delay(10);
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(10);

    tsys01 temperatureSensor = tsys01();
    ms5837 depthSensor = ms5837();
    double temp = temperatureSensor.getTemp();
    double depth = depthSensor.getDepth();

    unsigned long previousTime = 0, currentTime = 0;
    unsigned long fixEphemeridesTime = 0;
    int idRecord = 0;
    int count = 0;

    currentTime = getTime(); // Init Current TIme

    while (millis() < start + TIME_GPS_END * 1000 && (!gpsOK || !timeOK || !recordsOK) && fixEphemeridesOK)
    {
        if (GPSSerial.available() > 0 && gps.encode(GPSSerial.read()))
        {
            log_d("Count %d, IdRecord %d", count, idRecord);

            if (gps.date.isValid() && gps.time.isValid())
            {
                log_v("Date: %d/%d/%d\tHour: %d:%d:%d",
                      gps.date.day(), gps.date.month(), gps.date.year(),
                      gps.time.hour(), gps.time.minute(), gps.time.second());

                TimeElements gpsTime = {
                    (uint8_t)gps.time.second(),
                    (uint8_t)gps.time.minute(),
                    (uint8_t)gps.time.hour(),
                    0,
                    (uint8_t)gps.date.day(),
                    (uint8_t)gps.date.month(),
                    (uint8_t)(gps.date.year() - 1970)};
                setTime(makeTime(gpsTime));

                if (year() > 2020 && year() < 2050)
                {
                    timeOK = true;
                    pos.dateTime = now();
                    log_d("DateTime: %ld\tNow:%ld", pos.dateTime, now());
                }
            }
            if (gps.location.isValid())
            {
                log_d("Position: %f , %f", getLat(), getLng());
                if (gpsOK == false) // si la position n'avait jamais été obtenue, on sauvegarde uniquement la première pour la sortie de l'eau
                {
                    pos.Lat = (lat)gps.location.lat();
                    pos.Lng = (lng)gps.location.lng();
                    gpsOK = true;
                }
                fixEphemeridesOK = false; // passe à false pour garder le gps actif pendant au moins x minutes
                fixEphemeridesTime = millis();
            }

            if (!fixEphemeridesOK)
            {
                if (millis() - fixEphemeridesTime > TIME_DIVE_EPHEMERIDES_FIX * 1000)
                {
                    digitalWrite(GPIO_GPS_POWER, HIGH); // on éteint la led GPS
                    fixEphemeridesOK = true;            // passe à true pour terminer la plongée si possible
                }
            }

            if (millis() - start < TIME_GPS_END * 1000) // on enregistre que pendant la durée de TIME_GPS_END
            {
                depth = depthSensor.getDepth();
                currentTime = getTime();

                if (currentTime != previousTime && !recordsOK) // check if time changed
                {
                    count++;
                    previousTime = currentTime; // reset previous time
                }

                if (count >= TIME_GPS_RECORDS && !recordsOK) // if new records required and array not full
                {
                    count = 0;

                    // save temp and depth
                    temp = temperatureSensor.getTemp();
                    records[idRecord].Depth = depth;
                    records[idRecord].Temp = temp;
                    records[idRecord].Time = (idRecord + 1) * TIME_GPS_RECORDS + oldTime;
                    idRecord++;
                    if (idRecord == recordsLength)
                        recordsOK = true;
                }
            }
        }
    }
    digitalWrite(GPIO_LED2, LOW); // turn syn led off when gps connected
    log_v("DateTime: %ld\tNow:%ld", pos.dateTime, now());

    if (timeOK && gpsOK) // save if datetime and position is ok
    {
        pos.valid = true;
        log_d("POsition and dateTime valid");
    }

    return pos;
}

int GNSS::getEphemerides()
{
    pinMode(GPIO_GPS_POWER, OUTPUT);
    digitalWrite(GPIO_GPS_POWER, LOW);
    pinMode(GPIO_SENSOR_POWER, OUTPUT);
    digitalWrite(GPIO_SENSOR_POWER, LOW);
    pinMode(GPIO_LED2, OUTPUT);
    digitalWrite(GPIO_LED2, LOW);

    GPSSerial.begin(9600);
    delay(500);

    unsigned long start = millis();
    unsigned long maxDuration = TIME_GPS_EPHEMERIDE_MAX * 1000UL;
    const unsigned long fixDuration = TIME_GPS_EPHEMERIDE_FIX * 1000UL;
    bool ledState = HIGH;

    bool fixAcquired = false;
    unsigned long fixStartMillis = 0, blinkLedMillis = 0, currentMillis = 0;
    String sentence;
    int nbSat = 0;

    while (millis() - start < maxDuration)
    {
        while (GPSSerial.available() > 0)
        {
            char c = GPSSerial.read();
            sentence += c;

            // Détection de fin de ligne => on traite la trame complète
            if (c == '\n')
            {
                // On cible la trame GPGGA pour extraire le nombre de satellites
                if (sentence.startsWith("$GPGGA"))
                {
                    // Découpage des champs séparés par des virgules
                    int fieldIndex = 0;
                    String fields[15];
                    int startIdx = 0;
                    for (int i = 0; i < sentence.length(); i++)
                    {
                        if (sentence.charAt(i) == ',' || sentence.charAt(i) == '\n')
                        {
                            fields[fieldIndex++] = sentence.substring(startIdx, i);
                            startIdx = i + 1;
                        }
                    }

                    // Le 9e champ (index 8) indique le nombre de satellites suivis
                    if (fieldIndex >= 8)
                    {
                        nbSat = fields[7].toInt();
                        log_d("Nb satellites : %d", nbSat);

                        // Si on détecte > 5 satellites
                        if (nbSat > 4)
                        {
                            // Première fois qu'on dépasse 5 satellites ?
                            if (!fixAcquired)
                            {
                                fixAcquired = true;
                                fixStartMillis = millis();
                                log_d("5 satellites détectés, démarrage du chrono Ephemerides.");
                                digitalWrite(GPIO_LED2, ledState);
                            }
                            else
                            {
                                currentMillis = millis();
                                // blink led
                                if (currentMillis - blinkLedMillis >= 500)
                                {
                                    ledState = (ledState == LOW) ? HIGH : LOW;
                                    digitalWrite(GPIO_LED2, ledState);
                                    blinkLedMillis = currentMillis;
                                }

                                // Chrono déjà lancé : on vérifie si 15 min se sont écoulées
                                if (currentMillis - fixStartMillis >= fixDuration)
                                {
                                    // On éteint le GPS après 15 minutes avec au moins 5 satellites
                                    log_d("Chrono écoulé avec 5 satellites, extinction du GPS.");
                                    digitalWrite(GPIO_GPS_POWER, HIGH);
                                    digitalWrite(GPIO_SENSOR_POWER, HIGH);
                                    return nbSat;
                                }
                            }
                        }
                    }
                }
                // On réinitialise la chaîne pour la prochaine trame
                sentence = "";
            }
        }
    }

    // Si on sort de la boucle, c’est qu’on a atteint TIME_GPS_EPHEMERIDE sans maintenir 15 min à > 5 satellites
    log_d("Fin de la période TIME_GPS_EPHEMERIDE sans atteindre 15 min à >5 satellites.");
    digitalWrite(GPIO_GPS_POWER, HIGH); // Extinction du GPS
    digitalWrite(GPIO_SENSOR_POWER, HIGH);

    return -3;
}
