#include <Navigation/GNSS.hpp>
#include <Types.hpp>
#include <Settings.hpp>

GNSS::GNSS()
{
    digitalWrite(GPIO_LED2,HIGH);
    pinMode(GPIO_GPS_POWER, OUTPUT);
    digitalWrite(GPIO_GPS_POWER, LOW);
    GPSSerial.begin(9600);
    delay(2000); // TODO this needs to be more dynamic
    unsigned long start = millis();
    bool gpsOK = false;

    while (millis() < start + TIME_GPS * 1000 && !gpsOK)
    {
        if (GPSSerial.available() > 0 && gps.encode(GPSSerial.read()))
        {
            if (gps.date.isValid() && gps.time.isValid())
            {
                log_d("Date: %d/%d/%d", gps.date.day(), gps.date.month(), gps.date.year());
                log_d("Hour: %d:%d:%d", gps.time.hour(), gps.time.minute(), gps.time.second());

                TimeElements gpsTime = {
                    (uint8_t)gps.time.second(),
                    (uint8_t)gps.time.minute(),
                    (uint8_t)gps.time.hour(),
                    0,
                    (uint8_t)gps.date.day(),
                    (uint8_t)gps.date.month(),
                    (uint8_t)(gps.date.year() - 1970)};
                setTime(makeTime(gpsTime));
            }
            if (gps.location.isValid())
            {
                log_d("Position: %f , %f", getLat(), getLng());
                gpsOK=true;
            }

        }
    }
    digitalWrite(GPIO_LED2,LOW);//turn syn led off when gps connected

}

lat GNSS::getLat()
{
    parse();
    return (lat)gps.location.lat();
}

lng GNSS::getLng()
{
    parse();
    return (lng)gps.location.lng();
}

void GNSS::parse()
{
    unsigned long start = millis();
    while (GPSSerial.available() > 0 && millis() < start + 1000)
    {
        if (gps.encode(GPSSerial.read()))
        {
            if (gps.date.isValid() && gps.time.isValid())
            {
                TimeElements gpsTime = {
                    (uint8_t)gps.time.second(),
                    (uint8_t)gps.time.minute(),
                    (uint8_t)gps.time.hour(),
                    0,
                    (uint8_t)gps.date.day(),
                    (uint8_t)gps.date.month(),
                    (uint8_t)(gps.date.year() - 1970)};
                setTime(makeTime(gpsTime));
            }
        }
    }
}