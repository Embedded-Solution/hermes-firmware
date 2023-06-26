#ifndef WIFIMANAGER_HPP
#define WIFIMANAGER_HPP
#include <Arduino.h>
#include <Wire.h>
#include <AutoConnect.h>
#include <Settings.hpp>
#include <secret.hpp>
#include <ESPmDNS.h>

/// @brief Start Remora AP for WIFI configuration
/// @param sd
void startPortal(SecureDigital sd);

/// @brief Delete all saved credentials 
void deleteAllCredentials(void);

 // AutoConnectAux as a custom Web page.
const char page[] PROGMEM = R"raw(
  {
    "title": "Home",
    "uri": "/home",
    "menu": true,
    "element": [
      {
        "name": "header",
        "type": "ACText",
        "value": "<h2>Remora Wifi settings</h2>",
        "style": "text-align:center;color:#2f4f4f;padding:10px;"
      },
      {
        "name": "caption",
        "type": "ACText",
        "value": "Configure new Wifi Connexion<br>",
        "style": "font-family:serif;color:#4682b4;"
      },
      {
        "name": "new",
        "type": "ACSubmit",
        "value": "New",
        "uri": "/_ac/config"
      },
      {
        "name": "manage",
        "type": "ACSubmit",
        "value": "Manage",
        "uri": "/_ac/open"
      }
    ]
  }
)raw";

#endif
