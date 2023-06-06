#pragma once

#include <ArduinoJson.h>

class SdConfigFileReader
{
protected:
    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    // Done for the tec config files (4096)
    StaticJsonDocument<4096> doc;
    virtual bool loadConfiguration(const char* fileName) final;

public:
    void printFile(const char *fileName);
    virtual bool parseConfiguration(const char* fileName) = 0;
};
