#pragma once

#include <cstdint>

#ifdef TEENSYDUINO
#include <NativeEthernet.h>
#else
#include <SPI.h>
#include <Ethernet.h>
#endif

#include "CommService.h"

class IpCommsService : public CommsService
{
protected:
    static bool hardwareConfigurationDone;
    static byte mac[6];
    IPAddress ip;
    void getTeensyMacAddr(uint8_t *mac);

public: 
    bool initializeEnetIface(uint16_t);
    bool Status() { return this->commsServiceStatus; };
};