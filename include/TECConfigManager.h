#pragma once
#include <cinttypes>
#include <vector>
#include <string>
#include "SDConfigFileReader.h"
#include "teensy41_device.h"


struct TECConfig
{
    uint8_t tecNo;
    uint8_t boardNo;
    uint8_t channelNo;
};

struct TECControllerConfig
{
    uint8_t ip[4];
    uint16_t port;
    uint8_t boardNo;
    std::vector<TECConfig*> tecConfigs;
};

class TECConfigManager : SdConfigFileReader
{
public:
    TECControllerConfig cfg;
    bool parseConfiguration(const char *fileName) override;
    uint8_t getThisBoardNo(){return controllerBoardNo;}
private:
    uint8_t controllerBoardNo;
};