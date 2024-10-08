#include "WifiManager.hpp"
#include <esp_task_wdt.h> // Nécessaire pour gérer le Task Watchdog Timer (TWDT)

void WifiManager::startPortal(SecureDigital sd)
{
  WebServer Server;
  AutoConnect Portal(Server);

  log_v("starting config portal...");

  AutoConnectConfig acConfig("Remora Config", "cousteau", 0, AUTOCONNECT_AP_CH);
  acConfig.hostName = String("Remora");
  acConfig.bootUri = AC_ONBOOTURI_HOME;
  acConfig.homeUri = "/home";
  acConfig.autoReconnect = true;
  acConfig.autoReset = false;
  // acConfig.portalTimeout = 15 * 60 * 1000;
  acConfig.title = "Remora Config";
  acConfig.ticker = true;
  acConfig.tickerPort = GPIO_LED3;
  acConfig.tickerOn = LOW;
  acConfig.menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_OPENSSIDS | AC_MENUITEM_DELETESSID;

  Portal.config(acConfig);

  Portal.load(page);

  SPIFFS.end();

  esp_task_wdt_init(60, false); // 60 secondes, false pour désactiver le WDT pour toutes les tâches
  TaskHandle_t TaskLedBattery;
  xTaskCreatePinnedToCore(TaskLedBatteryCode, "TaskLedBattery", 10000, NULL, 0, &TaskLedBattery, 0);

  Portal.begin();
  if (MDNS.begin("remora"))
  {
    MDNS.addService("http", "tcp", 80);
  }
  long previous = -1000000000;

  while (WiFi.status() == WL_DISCONNECTED)
  {
    Portal.handleClient();
  }
  log_i("Adresse IP : %s", WiFi.localIP().toString().c_str());

  // detach interrupt to keep remora alive during upload and ota process even if usb is disconnected
  detachInterrupt(GPIO_VCC_SENSE);

  log_v("Wifi connected");

  esp_task_wdt_init(60, true); // 60 secondes, true pour réactiver le WDT pour toutes les tâches

  log_v("Start upload dives");

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

void WifiManager::deleteAllCredentials(void)
{
  AutoConnectCredential credential;
  station_config_t config;
  uint8_t ent = credential.entries();

  Serial.println("AutoConnectCredential deleting");
  if (ent)
    Serial.printf("Available %d entries.\n", ent);
  else
  {
    Serial.println("No credentials saved.");
    return;
  }

  while (ent--)
  {
    credential.load((int8_t)0, &config);
    if (credential.del((const char *)&config.ssid[0]))
      Serial.printf("%s deleted.\n", (const char *)config.ssid);
    else
      Serial.printf("%s failed to delete.\n", (const char *)config.ssid);
  }
}

bool WifiManager::checkDeleteCredentials(void)
{
  bool exit = false;
  unsigned long timer = millis();
  while (exit == false && (millis() - timer) / 1000 <= TIMER_DEL_CREDENTIALS)
  {
    if (digitalRead(GPIO_CONFIG) == false) // if button released, exit = true
      exit = true;
  }

  if (exit == true) // if config button released before timer end, leave without delete credentials
  {
    log_d("Leave without deleting credentials");
    return false;
  }
  else // if config button still pressed, delete credentials and blink leds
  {
    log_d("Delete all credentials ");
    deleteAllCredentials();

    // blink leds
    for (int i = 0; i < 5; i++)
    {
      digitalWrite(GPIO_LED1, HIGH);
      digitalWrite(GPIO_LED2, HIGH);
      digitalWrite(GPIO_LED3, HIGH);
      digitalWrite(GPIO_LED4, HIGH);
      delay(300);
      digitalWrite(GPIO_LED1, LOW);
      digitalWrite(GPIO_LED2, LOW);
      digitalWrite(GPIO_LED3, LOW);
      digitalWrite(GPIO_LED4, LOW);
      delay(300);
    }

    return true;
  }
}
