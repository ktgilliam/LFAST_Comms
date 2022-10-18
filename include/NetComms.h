#pragma once

#include <cstdint>
#include <NativeEthernet.h>
#include <list>

#include "CommService.h"

#define PORT 4400


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
        
        // std::vector<NetCommsMessage *> netMessageQueue;
        // std::vector<EthernetClient> enetClients;
        // EthernetClient enetClients[MAX_CLIENTS];
        std::list<EthernetClient> enetClients;
        // uint8_t connectedClients;
        
    public:
        EthernetCommsService();

        // Overloaded functions:

        bool Status() { return this->commsServiceStatus; };
        // bool checkForNewMessages(){return false;}
        // bool checkForNewMessages(EthernetClient &client);
        bool checkForNewClients() override;
        // virtual void sendMessage(CommsMessage<int> msg){};
    };
}