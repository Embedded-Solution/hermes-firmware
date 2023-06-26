#include "WifiManager.hpp"

void startPortal(SecureDigital sd)
{
    WebServer Server;
    AutoConnect Portal(Server);

    log_v("starting config portal...");

    AutoConnectConfig acConfig("Remora Config", "cousteau", 0, AUTOCONNECT_AP_CH);
    acConfig.hostName = String("Remora");
    acConfig.homeUri = "/remora";
    acConfig.autoReconnect = true;
    acConfig.autoReset = false;
    acConfig.portalTimeout = 15 * 60 * 1000;
    acConfig.title = "Remora Config";
    acConfig.ticker = true;
    acConfig.tickerPort = GPIO_LED3;
    acConfig.tickerOn = LOW;
    acConfig.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_OPENSSIDS | AC_MENUITEM_DELETESSID;

    Portal.config(acConfig);

    Portal.load(page);

    SPIFFS.end();

    Portal.begin();
    if (MDNS.begin("remora"))
    {
        MDNS.addService("http", "tcp", 80);
    }
    long previous = -1000000000;

    TaskHandle_t TaskLedBattery;
    xTaskCreatePinnedToCore(TaskLedBatteryCode, "TaskLedBattery", 10000, NULL, 0, &TaskLedBattery, 0);

    while (WiFi.status() == WL_DISCONNECTED)
    {
        Portal.handleClient();
    }
    log_i("Adresse IP : %s", WiFi.localIP().toString().c_str());

    // detach interrupt to keep remora alive during upload and ota process even if usb is disconnected
    detachInterrupt(GPIO_VCC_SENSE);

    log_v("Wifi connected, start upload dives");

    while (WiFi.status() == WL_CONNECTED && digitalRead(GPIO_VCC_SENSE))
    {

        if (millis() - previous > TIME_UPLOAD_OTA * 1000) // retry upload and ota after a while
        {
            pinMode(GPIO_LED2, OUTPUT);
            digitalWrite(GPIO_LED2, HIGH);

            if (uploadDives(sd) != SUCCESS)
                log_e("Error after upload");

            log_v("Upload finished, start OTA");
            if (ota(sd) != SUCCESS)
                log_e("Error after OTA");

            digitalWrite(GPIO_LED2, LOW);
            log_v("OTA finished, waiting for usb disconnection");

            previous = millis(); // reset upload and ota retry timer
        }
        Portal.handleClient();
    }

    log_v("USB disconnected, go back to sleep");
    sleep(DEFAULT_SLEEP);
}


void deleteAllCredentials(void) {
  AutoConnectCredential credential;
  station_config_t config;
  uint8_t ent = credential.entries();

  while (ent--) {
    credential.load(0, &config);
    credential.del((const char*)&config.ssid[0]);
  }
}
