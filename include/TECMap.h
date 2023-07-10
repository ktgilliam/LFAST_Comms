#pragma once
#include <cinttypes>
#include <vector>

struct TECMapEntry
{
    uint8_t boxNo;
    uint8_t boardNo;
    uint8_t channelNo;
};

class TECMap
{
public:
    static const TECMapEntry &get(unsigned int tecNo)
    {
        if (tecNo > 0)
        {
            return map.at(tecNo - 1);
        }
        else
        {
            return defaultEntry;
        }
    }
    
private:
    static const TECMapEntry defaultEntry;
    static const std::vector<TECMapEntry> map;
};