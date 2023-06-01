#pragma once

#include <ArduinoJson.h>

class SdConfigFileReader
{
private:
    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    // Done for the tec config files (4096)
    StaticJsonDocument<4096> doc;

public:
    virtual void loadConfiguration() final;
    void printFile(const char *filename);
    virtual void parseConfiguration() = 0;
};
