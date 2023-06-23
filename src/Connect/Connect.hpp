#ifndef CONNECT_HPP
#define CONNECT_HPP
#include <Wake.hpp>
#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <AutoConnect.h>
#include <Storage/SecureDigital.hpp>
#include <Navigation/GNSS.hpp>
#include <Utils.hpp>
#include <Settings.hpp>
#include <ArduinoJson.h>
#include <Dive.hpp>
#include <secret.hpp>
#include <ESPmDNS.h>

#define SUCCESS 0
#define FIRMWARE_SIZE_ERROR -2
#define GET_FIRMWARE_ERROR -3
#define CONNECTION_ERROR -4
#define HTTP_BEGIN_ERROR -5

/// @brief Start Remora AP for WIFI configuration
/// @param sd
void startPortal(SecureDigital sd);

/// @brief Upload dives not yet send to recordURL
/// @param sd
/// @return 0 = success;-1 = sd card error;-2 = no index file
int uploadDives(SecureDigital sd);

/// @brief get firmware and install if newer
/// @return
int ota(SecureDigital sd);

/// @brief post dive metadatas
/// @param data json content
/// @return
long postMetadata(String data);

/// @brief post dive record data
/// @param data json content
/// @return
int postRecordData(String data, unsigned long id);

/// @brief complete silo with id return after post metadata
/// @param records silo  pointer
/// @param diveID ID return by database after post metadata
/// @return updated string
String updateId(String data, unsigned long bddID);
long checkId(String data);
int putEndTransfer(unsigned long bddID);

const int jsonSize = 27000;


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