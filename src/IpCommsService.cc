#include "IpCommsService.h"

bool LFAST::IpCommsService::hardwareConfigurationDone = false;
IPAddress parseIpAddress(byte *bytes);


// const IPAddress defaultIp = IPAddress(192, 168, 121, 177);
// const uint16_t defaultPort = DEFAULT_PORT;
byte LFAST::IpCommsService::mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};


void LFAST::IpCommsService::getTeensyMacAddr(uint8_t *mac)
{
    for (uint8_t by = 0; by < 2; by++)
        mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
    for (uint8_t by = 0; by < 4; by++)
        mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
}

bool LFAST::IpCommsService::initializeEnetIface(uint16_t _port)
{
    bool initResult = true;
    port = _port;

    if (!hardwareConfigurationDone)
    {
        getTeensyMacAddr(mac);
        // initialize the Ethernet device
        Ethernet.begin(mac, ip);
        // Ethernet.begin(mac, ip, myDns, gateway, subnet)

        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
            #if defined(TERMINAL_ENABLED)
            if (cli != nullptr)
                cli->printfDebugMessage("Ethernet PHY was not found.  Sorry, can't run without hardware. :(");
                #endif
            initResult = false;
        }

        if (Ethernet.linkStatus() == LinkOFF) 
        {
            cli->printfDebugMessage("Ethernet cable is not connected.");
        }
        hardwareConfigurationDone = true;
    }
    if (initResult)
        commsServiceStatus = true;
    return commsServiceStatus;
}