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
    uint8_t boxNo;
    uint8_t ip[4];
    uint16_t port;
    uint8_t boardNo;
    std::vector<TECConfig*> tecConfigs;
};

class TECConfigManager : SdConfigFileReader
{
public:
    bool parseConfiguration(const char *fileName) override;
    uint8_t getThisBoardNo(){return controllerBoardNo;}
    uint8_t getThisBoxNo(){return cfg.boxNo;}
    TECConfig* getTecConfig(uint8_t tecNo);

    TECControllerConfig cfg;
private:
    uint8_t controllerBoardNo;
};