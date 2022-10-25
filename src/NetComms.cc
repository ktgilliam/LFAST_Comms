///
///  @ Author: Kevin Gilliam
///  @ Create Time: 2022-09-07 15:54:35
///  @ Modified by: Kevin Gilliam
///  @ Modified time: 2022-09-08 12:53:34
///  @ Description:
///

#include "../include/NetComms.h"

#include <Arduino.h>

#ifdef TEENSYDUINO
#include <NativeEthernet.h>
#else
#include <SPI.h>
#include <Ethernet.h>
#endif

#include <array>
#include <tuple>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <stdlib.h>     /* atoi */

#include <vector>
#include <iterator>
// #include <regex>

// Initialize the Ethernet server library
// with the IP address and port you want to use

void stopDisconnectedClients();
void getTeensyMacAddr(uint8_t *mac);
IPAddress parseIpAddress(char *addrStr);

const IPAddress defaultIp = IPAddress(192, 168, 121, 177);
const uint16_t defaultPort = DEFAULT_PORT;

LFAST::EthernetCommsService::port = defaultPort;
IPAddress LFAST::EthernetCommsService::ip = defaultIp;
EthernetServer LFAST::EthernetCommsService::server = EthernetServer();

byte LFAST::EthernetCommsService::mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
EthernetCommsService(char *pIp, unsigned int port)
{
    FAST::EthernetCommsService::ip = parseIpAddress(pIp);
    LFAST::EthernetCommsService::port = port;
    EthernetCommsService();
}

LFAST::EthernetCommsService::EthernetCommsService()
{
    bool initResult = true;

    // Serial2.println("Initializing Ethernet... ");
    getTeensyMacAddr(mac);
    // initialize the Ethernet device
    Ethernet.begin(mac, ip);
    // Ethernet.begin(mac, ip, myDns, gateway, subnet)

    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        // Serial2.println("Ethernet PHY was not found.  Sorry, can't run without hardware. :(");
        initResult = false;
    }

    // Serial2.println("Checking Link...");
    if (Ethernet.linkStatus() == LinkOFF)
    {
        // Serial2.println("Ethernet cable is not connected.");
        initResult = false;
    }

    if (initResult)
    {
        // start listening for clients
        this->server.begin(PORT);
        // Serial2.print("Listening for connection on local IP: ");
        // Serial2.print(Ethernet.localIP());
        // Serial2.print("...");
    }
    this->commsServiceStatus = initResult;
}

bool LFAST::EthernetCommsService::checkForNewClients()
{
    bool newClientFlag = false;
    // check for any new client connecting, and say hello (before any incoming data)
    EthernetClient newClient = server.accept();
    if (newClient)
    {
        newClientFlag = true;
        // Serial2.printf("Connection # %d Made.\r\n", connections.size() + 1);
        // Once we "accept", the client is no longer tracked by EthernetServer
        // so we must store it into our list of clients
        enetClients.push_back(newClient);
        setupClientMessageBuffers(&enetClients.back());
    }
    return (newClientFlag);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// LOCAL/PRIVATE FUNCTIONS ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void LFAST::EthernetCommsService::getTeensyMacAddr(uint8_t *mac)
{
    for (uint8_t by = 0; by < 2; by++)
        mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
    for (uint8_t by = 0; by < 4; by++)
        mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
}

IPAddress parseIpAddress(char *addrStr)
{
    char *pch;
    uint16_t ipParts[] = {0,0,0,0};
    pch = strtok(addrStr, ".");
    uint16_t idx = 0;
    while (pch != NULL)
    {
        pch = std::strtok(NULL, ".");
        ipParts(idx++) = std::atoi(pch);
    }
    return IPAddress(ipParts[0], ipParts[1], ipParts[2], ipParts[3]);
}