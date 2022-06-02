#include <Connect.hpp>
#include <ArduinoJson.h>
#include <Dive.hpp>

int uploadDives(SecureDigital sd)
{
    bool error = false;
    StaticJsonDocument<1024> indexJson;
    sd = SecureDigital();

    String data;
    data = sd.readFile(indexPath);
    if (data == "")
    {
        Serial.println("Could not read index file to upload dives");
        return -1;
    }
    deserializeJson(indexJson, data);
    JsonObject root = indexJson.as<JsonObject>();
    for (JsonObject::iterator it = root.begin(); it != root.end(); ++it)
    {
        String ID = it->key().c_str();
        Serial.println(ID);

        JsonObject dive = indexJson[ID].as<JsonObject>();
        const int uploaded = dive["uploaded"];
        Serial.println(uploaded);
        if (uploaded != 0)
        {
            continue;
        }

        String metadata = sd.readFile("/" + ID + "/metadata.json");
        log_d("RECORDS = %s ", metadata);
        if (post(metadata, true) != 200) // post metadata
            error = true;
        else
            log_d("Metadata posted");

        int i = 0;
        String path = "/" + ID + "/silo0.json";

        String records = "";
        while (sd.findFile(path) == 0)
        {
            String records = sd.readFile(path);
            if (post(records) != 200) // post silos
                error = true;
            else
                log_d("Silo %d posted", i);
            i++;
            path = "/" + ID + "/silo" + i + ".json";
        }

        if (!error)
            dive["uploaded"] = 1;
    }

    String buffer;
    serializeJson(indexJson, buffer);

    return sd.writeFile(indexPath, buffer);
}

int post(String data, bool metadata)
{

    Serial.println(data);
    if ((WiFi.status() == WL_CONNECTED))
    {

        HTTPClient http;
        WiFiClientSecure client;
        // client.setCACert(test_root_ca);
        client.setInsecure();

        if (!http.begin(client, metadata ? metadataURL : recordURL))
        {
            log_d("BEGIN FAILED...");
        }

        http.addHeader("Content-Type", "application/json");
        int code = http.POST(data.c_str());
        log_d("HTTP RETURN = %d", code);

        // Disconnect
        http.end();

        return code;
    }
    else
    {
        Serial.println("****** NO WIFI!!");
        return -1;
    }
    return -2;
}

void startPortal(SecureDigital sd)
{
    WebServer Server;
    AutoConnect Portal(Server);

    Serial.printf("starting config portal...\n");
    AutoConnectConfig acConfig("Remora Config", "cousteau");
    // acConfig.hostName = String("remora");
    // acConfig.homeUri = "https://www.google.fr";
    acConfig.autoReconnect = true;
    acConfig.autoReset = false;
    acConfig.portalTimeout = 15 * 60 * 1000;
    acConfig.title = "Remora Config";
    acConfig.ticker = true;
    acConfig.tickerPort = GPIO_LED3;
    acConfig.tickerOn = LOW;
    Portal.config(acConfig);
    Portal.begin();
    bool error = false;

    while (WiFi.status() == WL_DISCONNECTED)
    {
        Portal.handleClient();
    }
    Serial.println(WiFi.localIP());

    pinMode(GPIO_LED1, OUTPUT);
    digitalWrite(GPIO_LED2, HIGH);

    log_d("Wifi connected, start upload dives");

    if (uploadDives(sd) != SUCCESS)
        error = true;

    log_d("Upload finished, start OTA");
    if (ota() != SUCCESS)
        error = true;

    pinMode(GPIO_LED1, OUTPUT);
    digitalWrite(GPIO_LED1, error);
    digitalWrite(GPIO_LED2, LOW);
    log_d("OTA finished, waiting for usb disconnection");

    pinMode(GPIO_VCC_SENSE, INPUT);
    while (WiFi.status() == WL_CONNECTED && digitalRead(GPIO_VCC_SENSE))
    {
        Portal.handleClient();
    }

    log_d("USB disconnected, go back to sleep");
    sleep(false);
}

int ota()
{
    String cloudFunction = "https://project-hermes.azurewebsites.net/api/Firmware";
    String firmwareName;
    HTTPClient http;
    String firmwareURL;
    Serial.println(WiFi.localIP());

    if (http.begin(cloudFunction))
    {
        if (http.GET() == 200)
        {
            firmwareName = http.getString();

            StaticJsonDocument<1024> firmwareJson;
            deserializeJson(firmwareJson, firmwareName);
            const char *name = firmwareJson["name"];
            String version = name;
            const char *url = firmwareJson["url"];
            firmwareURL = url;

            version.remove(0, 10);

            Serial.print("Firmware = "), Serial.println(version.toFloat());

            if (version.toFloat() <= FIRMWARE_VERSION)
            {
                http.end();
                Serial.printf("Will not update as I am version:%1.2f and you are offering version:%1.2f\n", version.toFloat(), FIRMWARE_VERSION);
                return OLD_FIRMWARE_ERROR;
            }
            else
            {
                // Start update
                size_t written = 0;
                size_t gotten = 1;
                if (http.begin(firmwareURL))
                {
                    if (http.GET() == 200)
                    {
                        gotten = http.getSize();
                        if (!Update.begin(gotten))
                        {
                            Serial.printf("Firmware file too big at %d\n", gotten);
                            http.end();
                            return FIRMWARE_SIZE_ERROR;
                        }
                        Serial.println("atempting to update...");
                        written = Update.writeStream(http.getStream());
                    }
                    http.end();
                }
                else
                {
                    Serial.println("could not get update file");
                    http.end();
                    return GET_FIRMWARE_ERROR;
                }

                if (written == gotten)
                {
                    Serial.println("Written : " + String(written) + " successfully");
                }
                else
                {
                    Serial.println("Written only : " + String(written) + "/" + String(gotten) + ". Retry?");
                }

                if (Update.end())
                {
                    Serial.println("OTA done!");
                    if (Update.isFinished())
                    {
                        Serial.println("Update successfully completed. Rebooting.");
                    }
                    else
                    {
                        Serial.println("Update not finished? Something went wrong!");
                    }
                }
                else
                {
                    Serial.println("Error Occurred. Error #: " + String(Update.getError()));
                }
            }
        }
        else
        {
            Serial.println("could not contact cloud function");
            http.end();
            return CONNECTION_ERROR;
        }
    }
    else
    {
        Serial.println("could not begin http client");
        return HTTP_BEGIN_ERROR;
    }
    http.end();
    return SUCCESS;
}