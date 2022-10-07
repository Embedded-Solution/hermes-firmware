#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#define FIRMWARE_VERSION 2.3

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP_STATIC 5 /* Time ESP32 will go to sleep (in seconds) between each static record */
#define TIME_DYNAMIC_MODE 1000 /*Time between two records in dynamic mode (ms)*/
#define minDepth    10          /* Min depth to validate dynamic dive */
#define maxCounter 10       /* 10 Number of No Water to end dynamic dive */
#define maxStaticCounter 2     /* 2 Number of No Water to end static dive */
#define WATER_TRIGGER 1500 //tension de détection de l'eau en mv entre 0 et 3300 (pour de l'eau douce mettre une valeur basse)
#define TIME_GPS    300 //research time gps (seconds)

#endif
