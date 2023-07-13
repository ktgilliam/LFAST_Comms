#include "TECConfigManager.h"

bool TECConfigManager::parseConfiguration(const char *fileName)
{
    bool cardPresent = loadConfiguration(fileName);
    if (cardPresent)
    {
        cfg.port = doc["Port"];
        uint8_t idx = 0;
        uint8_t minBoardNo = 99;
        for (JsonVariant v : doc["IP"].as<JsonArray>())
        {
            cfg.ip[idx++] = v.as<int>();
        }

        for (JsonVariant v : doc["TEC"].as<JsonArray>())
        {
            JsonObject obj = v.as<JsonObject>();
            TECConfig *newTec = new TECConfig();

            newTec->tecNo = obj["ID"];
            newTec->boardNo = obj["BOARD"];
            newTec->channelNo = obj["CHANNEL"];
            cfg.tecConfigs.push_back(newTec);
            if (newTec->boardNo < minBoardNo)
                minBoardNo = newTec->boardNo;
            // TEST_SERIAL.printf("%d : %d : %d\r\n", ID, BOARD, CHANNEL);
        }
        controllerBoardNo = minBoardNo;
        // TEST_SERIAL.printf("%d.%d.%d.%d:%d\r\n", cfg.ip[0], cfg.ip[1], cfg.ip[2], cfg.ip[3], cfg.port);
    }
    return cardPresent;
}

