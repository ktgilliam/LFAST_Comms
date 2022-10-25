#pragma once

#include <cstdint>
#include <NativeEthernet.h>
#include <list>

#include "CommService.h"

#define DEFAULT_PORT 4400


#define MAX_CLIENTS 4
namespace LFAST
{
    class EthernetCommsService : public CommsService
    {
    private:
        void getTeensyMacAddr(uint8_t *mac);
        static byte mac[6];
        static IPAddress ip;
        static EthernetServer server;
        static uint16_t port;
        
        std::list<EthernetClient> enetClients;
    public:
        EthernetCommsService();
        EthernetCommsService(char* ip, unsigned int port);
        // Overloaded functions:

        bool Status() { return this->commsServiceStatus; };
        bool checkForNewClients() override;
    };
}