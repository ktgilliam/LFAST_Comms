#pragma once

#include <cstdint>
#ifdef TEENSYDUINO
#include <NativeEthernet.h>
#else
#include <SPI.h>
#include <Ethernet.h>
#endif
#include <list>

#include "CommService.h"


#define MAX_CLIENTS 4
namespace LFAST
{
    class EthernetCommsService : public CommsService
    {
    protected:
        void getTeensyMacAddr(uint8_t *mac);
        static byte mac[6];
        static IPAddress ip;
        static EthernetServer server;
        static uint16_t port;

        std::list<EthernetClient> enetClients;
    public:
        EthernetCommsService();
        EthernetCommsService(byte *, uint16_t);

        bool initializeEnetIface();

        bool Status() { return this->commsServiceStatus; };
        bool checkForNewClients() override;
    };
}