#include "SDConfigFileReader.h"
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "teensy41_device.h"

// https://arduinojson.org/v6/example/config/

bool SdConfigFileReader::loadConfiguration(const char* fileName)
{
    bool success = true;
    // Serial.print("Initializing SD card...");
    if (!SD.begin(BUILTIN_SDCARD))
    {
        success = false;
    }

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File configFile = SD.open(fileName);
    if (configFile)
    {
        // read from the file until there's nothing else in it:
        while (configFile.available())
        {
            // Deserialize the JSON document
            ReadBufferingStream bufferedFile(configFile, 64);
            DeserializationError error = deserializeJson(doc, bufferedFile);
            if (error)
                TEST_SERIAL.println(FLASH_STR("Failed to read file, using default configuration"));
        }
        // close the file:
        configFile.close();
    }
    else
    {
        success = false;
        // if the file didn't open, print an error:
        // TEST_SERIAL.println("error opening test.txt");
    }
    return success;
}

// Prints the content of a file to the Serial
void SdConfigFileReader::printFile(const char *filename)
{
    // Open file for reading
    File file = SD.open(filename);
    if (!file)
    {
        TEST_SERIAL.println(FLASH_STR("Failed to read file"));
        return;
    }

    // Extract each characters by one by one
    while (file.available())
    {
        TEST_SERIAL.print((char)file.read());
    }
    TEST_SERIAL.println();

    // Close the file
    file.close();
}