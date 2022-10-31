#pragma once

#include <cstdint>
#include <list>


#ifdef TEENSYDUINO
#include <NativeEthernet.h>
#else
#include <SPI.h>
#include <Ethernet.h>
#endif


#include "CommService.h"


#define MAX_CLIENTS 4
namespace LFAST
{
    class TcpCommsService : public CommsService
    {
    protected:
    static bool hardwareConfigurationDone;
        void getTeensyMacAddr(uint8_t *mac);
        static byte mac[6];
        IPAddress ip;
        uint16_t port;
        EthernetServer *tcpServer;

        std::list<EthernetClient> enetClients;
    public:
        TcpCommsService();
        TcpCommsService(byte *, uint16_t);

        bool initializeEnetIface();

        bool Status() { return this->commsServiceStatus; };
        bool checkForNewClients() override;
    };
}