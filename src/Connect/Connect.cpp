#include "Connect.hpp"

int uploadDives(SecureDigital sd)
{
    long bddID = 0;
    bool error = false;
    int count = 0;
    bool postOK = false;

    DynamicJsonDocument indexJson(27000);

    sd = SecureDigital();

    String data;
    data = sd.readFile(indexPath);
    if (data == "")
    {
        log_e("Could not read index file to upload dives");
        return -2;
    }
    deserializeJson(indexJson, data);
    log_v("DATA = %s", data.c_str());
    JsonObject root = indexJson.as<JsonObject>();

    for (JsonObject::iterator it = root.begin(); it != root.end(); ++it)
    {

        error = false;
        String ID = it->key().c_str();
        log_v("ID = %s", ID.c_str());

        JsonObject dive = indexJson[ID].as<JsonObject>();
        const int uploaded = dive["uploaded"];
        log_v("Uploaded = %d\n", uploaded);

        if (uploaded != 0)
        {
            continue;
        }

        String path = "/" + ID + "/metadata.json";
        String metadata = sd.readFile(path);

        // check if metadata already uploaded
        bddID = 0;
        bddID = checkId(metadata);
        log_d("NEW DIVE TO UPLOAD, BDD ID = %d", bddID);
        /// /// Can be removed after beta tests //////
        if (bddID > 1000000000 || bddID < 0) // if bug in metadata bddid
        {
            DynamicJsonDocument dataJson(jsonSize);
            deserializeJson(dataJson, metadata);
            dataJson.remove("id");
            metadata = "";
            serializeJson(dataJson, metadata);
            bddID = 0;
        }
        ////////////////////////////////////////

        // if not, post metadata and get new id
        if (bddID == 0)
        {
            count = 0;
            while (bddID <= 0 && count < POST_RETRY)
            {
                count++;
                bddID = postMetadata(metadata);
                if (bddID > 0)
                {
                    // update metadata with bdd ID on SD card
                    sd.writeFile(path, updateId(metadata, bddID));
                }
            }
        }
        log_i("BDD ID = %i\n\n", bddID);

        if (bddID < 0)
        {
            error = true;
            log_e("Error during metadata upload");
        }
        else
        {
            log_i("Metadata posted, start post silos");

            if (uploadSilos(sd, ID, bddID) == true)
                error = true;
        }
        if (!error)
        {
            count = 0;
            postOK = false;
            while (postOK == false && count < POST_RETRY)
            {
                if (putEndTransfer(bddID) == 200)
                {
                    postOK = true;
                    // TODO : if saveUploadID always working, writing bddID in indexFile not necessary
                    dive["uploaded"] = bddID;
                    String buffer;
                    serializeJson(indexJson, buffer);
                    sd.writeFile(indexPath, buffer);
                    log_i("Dive %d fully upload", bddID);

                    Dive d(&sd);
                    d.saveUploadID(ID, bddID);
                }
                else
                {
                    count++;
                }
            }
        }
    }

    return error;
}

long postMetadata(String data)
{
    long id = 0;
    if ((WiFi.status() == WL_CONNECTED))
    {

        HTTPClient http;
        WiFiClientSecure client;
        client.setInsecure();

        if (!http.begin(client, metadataURL))
        {
            log_e("BEGIN FAILED...");
        }

        http.setAuthorization(user.c_str(), password.c_str());
        http.addHeader("Content-Type", "application/json");
        int code = http.POST(data.c_str());
        log_d("HTTP RETURN = %d", code);

        if (code != 200)
        {
            log_e("HTTP Response code: %s", http.getString().c_str());
            return -3;
        }
        else
        {
            String response = http.getString().c_str();

            // convert getString() into id
            StaticJsonDocument<1024> responseJson;
            deserializeJson(responseJson, response);
            id = responseJson["id"];
            log_d("ID RETURN = %d", id);

            return id;
        }

        // Disconnect
        http.end();
    }
    else
    {
        log_e("****** NO WIFI!!");
        return -1;
    }
    return -2;
}

int postRecordData(String data, unsigned long id)
{

    if ((WiFi.status() == WL_CONNECTED))
    {

        HTTPClient http;
        WiFiClientSecure client;
        client.setInsecure();

        if (!http.begin(client, recordURL))
        {
            log_e("BEGIN FAILED...");
        }

        http.setAuthorization(user.c_str(), password.c_str());
        http.addHeader("Content-Type", "application/json");
        int code = http.POST(data.c_str());
        log_d("HTTP RETURN = %d", code);
        // log_v("HTTP RETURN = %s", http.getString().c_str());

        // Disconnect
        http.end();

        return code;
    }
    else
    {
        log_e("****** NO WIFI!!");
        return -1;
    }
    return -2;
}

int uploadSilos(SecureDigital sd, String ID, long bddID)
{
    long siloID = 0;
    bool postOK = false, error = false;
    int i = 0, count = 0;
    String path = "/" + ID + "/silo0.json";
    String records = "";

    while (sd.findFile(path) == 0)
    {
        records = sd.readFile(path);

        if (records != "")
        {
            // check if silo already uploaded
            siloID = checkId(records);
            if (siloID == 0)
            {
                // add bdd ID before upload
                records = updateId(records, bddID);
                log_i("Try to post Silo %d", i);

                count = 0;
                postOK = false;
                while (postOK == false && count < POST_RETRY)
                {
                    if (postRecordData(records, bddID) != 200) // post silos
                    {
                        error = true;
                        log_e("Silo %d not posted", i);
                    }
                    else
                    {
                        postOK = true;
                        log_i("Silo %d posted", i);
                        // update silo with bdd ID on SD card
                        sd.writeFile(path, records);
                    }

                    count++;
                }
            }
        }
        else
            log_i("Silo %d empty, skipped", i);

        i++;
        path = "/" + ID + "/silo" + i + ".json";
    }
    return error;
}

int ota(SecureDigital sd)
{
    sd = SecureDigital();

    String firmwareName;
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    String updateURL;
    log_v("Adresse IP : %s", WiFi.localIP().toString().c_str());

    http.setAuthorization(user.c_str(), password.c_str());

    if (http.begin(firmwareURL))
    {
        if (http.GET() == 200)
        {
            firmwareName = http.getString();

            StaticJsonDocument<1024> firmwareJson;
            deserializeJson(firmwareJson, firmwareName);
            const char *name = firmwareJson["name"];
            String version = name;
            const char *url = firmwareJson["url"];
            updateURL = url;
            log_d("Name = %s", version.c_str());

            version.remove(0, 10);
            log_i("OTA firmware = %s\tRemora firmware = %s", version.c_str(), FIRMWARE_VERSION);

            if (versionCompare(version.c_str(), FIRMWARE_VERSION) > 0)
            {
                log_d("Remora firmware version smaller, start Update");

                // Start update
                size_t written = 0;
                size_t gotten = 1;
                if (http.begin(client, updateURL))
                {
                    if (http.GET() == 200)
                    {
                        gotten = http.getSize();
                        if (!Update.begin(gotten))
                        {
                            log_e("Firmware file too big at %d\n", gotten);
                            http.end();
                            return FIRMWARE_SIZE_ERROR;
                        }
                        log_v("atempting to update...");
                        written = Update.writeStream(http.getStream());
                    }
                    http.end();
                }
                else
                {
                    log_e("could not get update file");
                    http.end();
                    return GET_FIRMWARE_ERROR;
                }

                if (written == gotten)
                {
                    log_i("Written : %d successfully", written);
                }
                else
                {
                    log_e("Written only : %d/%d . Retry?", written, gotten);
                }

                if (Update.end())
                {
                    log_v("OTA done!");
                    if (Update.isFinished())
                    {
                        log_v("Update successfully completed. Rebooting.");
                        // Write version on SD card
                        String path = "/version.txt";

                        sd.writeFile(path, version);
                    }
                    else
                    {
                        log_e("Update not finished? Something went wrong!");
                    }
                }
                else
                {
                    log_e("Error Occurred. Error #: %d", Update.getError());
                }
            }
            else
            {
                http.end();
                log_i("Will not update as I am version:%s and you are offering version:%s\n", FIRMWARE_VERSION, version.c_str());
                return SUCCESS;
            }
        }
        else
        {
            log_e("could not contact cloud function");
            http.end();
            return CONNECTION_ERROR;
        }
    }
    else
    {
        log_e("could not begin http client");
        return HTTP_BEGIN_ERROR;
    }
    http.end();
    return SUCCESS;
}

String updateId(String data, unsigned long bddID)
{
    DynamicJsonDocument dataJson(jsonSize);
    deserializeJson(dataJson, data);
    dataJson["id"] = bddID;
    String returnJson = "";
    serializeJson(dataJson, returnJson);
    return returnJson;
}

long checkId(String data)
{
    unsigned long id = 0;
    DynamicJsonDocument dataJson(jsonSize);
    deserializeJson(dataJson, data);
    id = dataJson["id"];
    return id;
}

int putEndTransfer(unsigned long bddID)
{
    if ((WiFi.status() == WL_CONNECTED))
    {

        HTTPClient http;
        WiFiClientSecure client;
        client.setInsecure();

        if (!http.begin(client, endTansfertURL + bddID))
        {
            log_e("BEGIN FAILED...");
        }

        http.setAuthorization(user.c_str(), password.c_str());
        http.addHeader("Content-Type", "text/plain");
        int code = http.PUT(String(bddID).c_str());
        log_i("PUT RETURN = %d", code);

        // Disconnect
        http.end();
        return code;
    }
    else
    {
        log_e("****** NO WIFI!!");
        return -1;
    }
    return -2;
}
