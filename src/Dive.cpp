#include <Dive.hpp>
#include <Storage/Storage.hpp>

RTC_DATA_ATTR char savedID[100];
RTC_DATA_ATTR int staticCurrentRecords;
RTC_DATA_ATTR int staticOrder;

Dive::Dive(void) {}

Dive::Dive(Storage *s)
{
    storage = s;
}

void Dive::init()
{

    ID = "";
    currentRecords = 0;
    diveRecords = new Record[siloRecordSize];
    order = 0;
    metadata.ID = "";
    metadata.startTime = 0;
    metadata.endTime = 0;
    metadata.freq = 0;
    metadata.siloSize = 0;
    metadata.startLat = 0;
    metadata.startLng = 0;
    metadata.endLat = 0;
    metadata.endLng = 0;
    staticCurrentRecords = 0;
    staticOrder = 0;
}

void Dive::resetRecords()
{
    delete diveRecords;
    diveRecords = new Record[siloRecordSize];
}

String Dive::Start(long time, lat lat, lng lng, int freq, bool mode)
{
    init();
    ID = createID(time);
    saveId(ID);
    log_d("Start dynamic dive, ID = %s", ID.c_str());
    diveRecords = new Record[siloRecordSize];
    if (writeMetadataStart(time, lat, lng, freq, mode) == -1)
    {
        return "";
    }
    createIndex();
    //// Write battery level on SD Card only to debug offset  //////////
    String path = "/" + ID + "/battery.txt";
    storage->writeFile(path, (String)readBattery() + "\n");
    /////////////////////////////////////////////////////////////
    return ID;
}

String Dive::End(long time, lat lat, lng lng, bool mode)
{
    log_d("End dive, ID = %s", ID.c_str());

    //// Write battery level on SD Card only to debug offset  //////////
    String path = "/" + ID + "/battery.txt";
    storage->appendFile(path, (String)readBattery());
    /////////////////////////////////////////////////////////////

    if (mode == 0) // write partial silo if dynamic ode
        writeSilo(true, currentRecords);

    if (writeMetadataEnd(time, lat, lng) == -1)
    {
        return "";
    }
    return ID;
}

int Dive::NewRecord(Record r)
{
    log_v("%d :\tTime=%ld\tTemp=%2.3f\t Depth=%2.3f", currentRecords, r.Time, r.Temp, r.Depth);

    diveRecords[currentRecords] = r;
    currentRecords++;
    if (currentRecords == siloRecordSize)
    {
        if (writeSilo() == -1)
        {
            log_e("Error saving silo");
            return -1;
        }
        resetRecords();
        currentRecords = 0;
    }
    return 0;
}

int Dive::NewRecordStatic(Record r)
{
    diveRecords[0] = r;

    if (writeStaticRecord() == -1)
    {
        log_e("Error saving silo");
        return -1;
    }

    return 0;
}

int Dive::writeSilo(bool last, int currentRecord)
{
    log_d("CurrentRecord = %d\tSilo = %d", currentRecord, order);
    String path = "/" + ID + "/silo" + String(order) + ".json";

    DynamicJsonDocument jsonSilo(siloByteSize);

    jsonSilo["diveId"] = ID;
    order++;

    JsonArray records = jsonSilo.createNestedArray("records");

    for (int i = 0; i < (last == true ? currentRecord : siloRecordSize); i++)
    {
        JsonArray record = records.createNestedArray();
        record.add((float)((int)(diveRecords[i].Temp * 100.0)) / 100.0);
        record.add((float)((int)(diveRecords[i].Depth * 100.0)) / 100.0);
        record.add(diveRecords[i].Time);
    }

    String buffer;
    serializeJson(jsonSilo, buffer);
    log_v("Free heap = %ld", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    return storage->writeFile(path, buffer);
}

int Dive::writeStaticRecord()
{
    ID = getID();

    String path = "/" + ID + "/silo" + String(staticOrder) + ".json";

    log_v("Current Records = %d", staticCurrentRecords);
    log_v("Order = %d", staticOrder);

    staticCurrentRecords++;
    // Change silo number if enough records
    if (staticCurrentRecords == siloRecordSize)
    {
        staticOrder++;
        staticCurrentRecords = 0;
    }

    DynamicJsonDocument jsonSilo(siloByteSize);

    // this should only happen to a new dive record
    if (storage->findFile(path) == -1)
    {
        jsonSilo["diveId"] = ID;

        JsonArray records = jsonSilo.createNestedArray("records");

        JsonArray record = records.createNestedArray();
        record.add(diveRecords[0].Temp);
        record.add(diveRecords[0].Depth);
        record.add(diveRecords[0].Time);
    }
    else
    {
        String records = storage->readFile(path);
        if (records == "")
        {
            log_e("Could not read previous ID records file");
            return -1;
        }
        else
        {
            deserializeJson(jsonSilo, records);

            JsonArray record = jsonSilo["records"].createNestedArray();
            record.add(diveRecords[0].Temp);
            record.add(diveRecords[0].Depth);
            record.add(diveRecords[0].Time);
        }
    }

    String buffer;
    serializeJson(jsonSilo, buffer);

    return storage->writeFile(path, buffer);
}

int Dive::writeMetadataStart(long time, double lat, double lng, int freq, bool mode)
{
    StaticJsonDocument<1024> mdata;
    storage->makeDirectory("/" + ID);
    String path = "/" + ID + "/metadata.json";

    mdata["deviceId"] = remoraID();
    mdata["diveId"] = ID;
    char buf[50];
    sprintf(buf, "%s - %1.2f", (mode == 1 ? "Static" : "Dynamic"), FIRMWARE_VERSION);
    mdata["mode"] = buf;
    mdata["startTime"] = time;
    mdata["startLat"] = lat;
    mdata["startLng"] = lng;
    mdata["freq"] = freq;

    String buffer;
    serializeJson(mdata, buffer);
    return storage->writeFile(path, buffer);
}

int Dive::writeMetadataEnd(long time, double lat, double lng)
{
    updateIndex(ID);

    StaticJsonDocument<1024> mdata;
    String path = "/" + ID + "/metadata.json";

    String data;
    data = storage->readFile(path);
    if (data == "")
    {
        log_e("Failed to read metadata to save dive!");
        return -1;
    }

    deserializeJson(mdata, data);

    mdata["endTime"] = time;
    mdata["endLat"] = lat;
    mdata["endLng"] = lng;

    String buffer;
    serializeJson(mdata, buffer);
    return storage->writeFile(path, buffer);
}

int Dive::createIndex()
{
    // this should only happen to a new device
    if (storage->findFile(indexPath) == -1)
    {
        DynamicJsonDocument index(indexByteSize);
        // index[ID] = 0;

        JsonObject dive = index.createNestedObject(ID);
        dive["uploaded"] = -1;

        String buffer;
        serializeJson(index, buffer);

        return storage->writeFile(indexPath, buffer);
    }
    else
    {
        String index = storage->readFile(indexPath);
        if (index == "")
        {
            log_e("Could not read index file");
            return -1;
        }
        else
        {
            DynamicJsonDocument newIndex(indexByteSize);

            deserializeJson(newIndex, index);
            JsonObject dive = newIndex.createNestedObject(ID);
            dive["uploaded"] = -1;

            String buffer;
            serializeJson(newIndex, buffer);

            return storage->writeFile(indexPath, buffer);
        }
    }
}

int Dive::updateIndex(String updatedID)
{

    String index = storage->readFile(indexPath);
    if (index == "")
    {
        log_e("Could not read index file");
        return -1;
    }
    else
    {
        DynamicJsonDocument newIndex(indexByteSize);

        deserializeJson(newIndex, index);
        JsonObject dive = newIndex.createNestedObject(updatedID);
        dive["uploaded"] = 0;

        String buffer;
        serializeJson(newIndex, buffer);

        return storage->writeFile(indexPath, buffer);
    }
}

int Dive::deleteIndex(String deletedID)
{

    String index = storage->readFile(indexPath);
    if (index == "")
    {
        log_e("Could not read index file");
        return -1;
    }
    else
    {
        DynamicJsonDocument newIndex(indexByteSize);

        deserializeJson(newIndex, index);
        newIndex.remove(deletedID);

        String buffer;
        serializeJson(newIndex, buffer);

        return storage->writeFile(indexPath, buffer);
    }
}

int Dive::checkIndex()
{
    DynamicJsonDocument indexJson(27000);

    String data;
    data = storage->readFile(indexPath);
    if (data == "")
    {
        log_e("Could not read index file to upload dives");
        return -2;
    }
    deserializeJson(indexJson, data);
    JsonObject root = indexJson.as<JsonObject>();

    for (JsonObject::iterator it = root.begin(); it != root.end(); ++it)
    {

        String ID = it->key().c_str();
        log_v("ID = %s", ID.c_str());

        JsonObject dive = indexJson[ID].as<JsonObject>();
        const int uploaded = dive["uploaded"];
        log_v("Uploaded = %d", uploaded);

        // if diveId can't be uploaded (-1), check if there is data and at least one valid GPS, datetime and min depth
        if (uploaded != -1)
        {
            log_v("Dive %s already uploaded", ID.c_str());
            continue;
        }
        log_d("Dive %s not uploaded", ID.c_str());

        // check if there is data.
        if (storage->checkDirectory("/" + ID) != 0)
        {
            log_e("Dive %s empty", ID.c_str());
            continue;
        }

        // check if there is metadata start

        StaticJsonDocument<1024> mdata;
        String path = "/" + ID + "/metadata.json";

        String metadata;
        metadata = storage->readFile(path);

        if (metadata == "")
        {
            log_e("Failed to read metadata");
            continue;
        }

        deserializeJson(mdata, metadata);
        bool validStart = false, validEnd = false, validDive = false;

        long startTime = (long)mdata["startTime"];
        double startLat = (double)mdata["startLat"];
        double startLng = (double)mdata["startLng"];
        long endTime = (long)mdata["endTime"];
        double endLat = (double)mdata["endLat"];
        double endLng = (double)mdata["endLng"];

        // Check START gps and timestamp
        if ((startTime > 1640991600 && startTime < 4163871600) && (startLat != 0 && startLng != 0))
        {
            validStart = true;
            log_d("ValidStart ");
        }
        // Check au moins une position gps de bonne
        if ((endTime > 1640991600 && endTime < 4163871600) && (endLat != 0 && endLng != 0))
        {
            validEnd = true;
            log_d("ValidEnd ");
        }

        if (validStart || validEnd)
        {
            // vérifié si la plongée contient des silos
            int siloNumber = 0;
            path = "/" + ID + "/silo" + siloNumber + ".json";
            String dataTxt = "";

            while (storage->findFile(path) == 0 && !validDive)
            {
                dataTxt = storage->readFile(path);

                if (dataTxt != "")
                {
                    DynamicJsonDocument dataJson(jsonSize);
                    deserializeJson(dataJson, dataTxt);
                    int i = 0;
                    while (i < 100)
                    {

                        double value = (double)dataJson["records"][i][1];

                        if (value > MIN_DEPTH_VALID_DIVE)
                        {
                            log_d("ValidDive ");
                            validDive = true;
                            break;
                        }

                        i++;
                    }

                    // verifier si la plongée à atteint la profondeur mini

                    // si aucun des 2 n'est valide, on ne permet pas l'uplaod et on supprime la plongée définitivement (index file et datas )
                }
                siloNumber++;
                path = "/" + ID + "/silo" + siloNumber + ".json";
            }

            if (validDive) // si l'un des 2 est valide, on permet l'upload
            {
                updateIndex(ID);
                log_d("Dive %s back to upload", ID.c_str());
            }
            else
            {
                // TODO change index file with value -2 to not check second time (or delete from index file)
            }
        }
        else
        {
            log_v("Dive %s, metadata not valid", ID.c_str());
        }
    }
}

void Dive::saveId(String ID)
{
    ID.toCharArray(savedID, ID.length() + 1);
}

String Dive::getID()
{
    return String(savedID);
}

String Dive::createID(long time)
{
    byte shaResult[32];
    WiFi.mode(WIFI_MODE_STA);
    String unhashed_id = String(time) + WiFi.macAddress() + String(esp_random());
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

    const size_t payloadLength = strlen(unhashed_id.c_str());

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char *)unhashed_id.c_str(), payloadLength);
    mbedtls_md_finish(&ctx, shaResult);
    mbedtls_md_free(&ctx);

    String hash;
    for (int i = 0; i < sizeof(shaResult); i++)
    {
        char str[3];

        sprintf(str, "%02x", (int)shaResult[i]);
        hash = hash + str;
    }

    return hash;
}

void Dive::deleteID(String ID)
{
    int i = 0;
    String pathRecords;
    do
    {
        pathRecords = "/" + ID + "/silo" + i + ".json";
        i++;
    } while (storage->deleteFile(pathRecords) == 0); // delete silo files

    pathRecords = "/" + ID + "/metadata.json";
    storage->deleteFile(pathRecords); // delete metadata file

    pathRecords = "/" + ID + "/battery.txt";
    storage->deleteFile(pathRecords); // delete battery file

    String path = "/" + ID;
    storage->removeDirectory(path); // remove directory
    deleteIndex(ID);
}