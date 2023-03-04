#pragma once

#include <cstdint>

#ifdef TEENSYDUINO
#include <NativeEthernet.h>
#else
#include <SPI.h>
#include <Ethernet.h>
#endif

#include "CommsService.h"

namespace LFAST
{
class IpCommsService : public CommsService
{
protected:
    static bool hardwareConfigurationDone;
    static byte mac[6];
    IPAddress ip;
    void getTeensyMacAddr(uint8_t *mac);
    uint16_t port;
public: 
    virtual bool initializeEnetIface(uint16_t);
    bool Status() { return this->commsServiceStatus; };
};
}