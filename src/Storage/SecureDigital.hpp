#ifndef SECUREDIGITAL_HPP
#define SECUREDIGITAL_HPP

#include <SD.h>
#include <FS.h>

#include <Storage/Storage.hpp>
#include <hal/remora-hal.h>

using namespace std;

class SecureDigital : public Storage
{
public:
    SecureDigital()
    {

        powerSDcard();

        SPI.begin(18, 19, 23);
        delay(10);
        if (!SD.begin(5))
        {
            log_e("Card Mount Failed");
            return;
        }
        uint8_t cardType = SD.cardType();

        if (cardType == CARD_NONE)
        {
            log_e("No SD card attached");
            return;
        }

        log_v("SD Card Type: ");
        if (cardType == CARD_MMC)
        {
            log_v("MMC");
        }
        else if (cardType == CARD_SD)
        {
            log_v("SDSC");
        }
        else if (cardType == CARD_SDHC)
        {
            log_v("SDHC");
        }
        else
        {
            log_v("UNKNOWN");
        }

        log_v("SD Card Size: %lluMB\n", SD.cardSize() / (1024 * 1024));
        m_ready = true;
    };

    bool ready(void)
    {
        return m_ready;
    }

    int makeDirectory(String path)
    {
        powerSDcard();

        if (SD.mkdir(path.c_str()))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    int checkDirectory(String path)
    {
        powerSDcard();

        if (SD.exists(path.c_str()))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    int removeDirectory(String path)
    {
        powerSDcard();

        if (SD.rmdir(path.c_str()))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    String readFile(String path)
    {
        powerSDcard();

        String data;
        File file = SD.open(path);
        if (!file)
        {
            return "";
        }

        while (file.available())
        {
            data = data + file.readString();
        }

        file.close();

        return data;
    }

    int writeFile(String path, String data)
    {
        powerSDcard();

        File file = SD.open(path, FILE_WRITE);
        if (!file)
        {
            return -1;
        }

        if (file.print(data))
        {
            file.close();
            return 0;
        }
        else
        {
            file.close();
            return -1;
        }
    }

    int appendFile(String path, String data)
    {
        powerSDcard();

        File file = SD.open(path, FILE_APPEND);
        if (!file)
        {
            return -1;
        }

        if (file.print(data))
        {
            file.close();
            return 0;
        }
        else
        {
            file.close();
            return -1;
        }
    }

    int renameFile(String pathA, String pathB)
    {
        powerSDcard();

        if (SD.rename(pathA, pathB))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    int deleteFile(String path)
    {
        powerSDcard();

        if (SD.remove(path))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    int findFile(String path)
    {
        powerSDcard();

        if (SD.exists(path))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    int touchFile(String path)
    {
        powerSDcard();

        if (SD.exists(path))
        {
            return 0;
        }
        else
        {
            File file = SD.open(path, FILE_WRITE);
            file.close();
            return 1;
        }
    }

private:
    bool m_ready = false;
    void powerSDcard()
    {
        pinMode(GPIO_3V3_EN, OUTPUT);
        digitalWrite(GPIO_3V3_EN, LOW);
        pinMode(GPIO_SD_EN, OUTPUT);
        digitalWrite(GPIO_SD_EN, LOW);
    }
};

#endif