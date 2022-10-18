///
///  @ Author: Kevin Gilliam
///  @ Create Time: 2022-09-07 15:54:35
///  @ Modified by: Kevin Gilliam
///  @ Modified time: 2022-09-08 12:53:34
///  @ Description:
///

#include "../include/NetComms.h"

#include <Arduino.h>

#ifdef TEENSY41
#include <NativeEthernet.h>
#else
#include <SPI.h>
#include <Ethernet.h>
#endif


#include <debug.h>
#include <array>
#include <tuple>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>

#include <vector>
#include <iterator>
// #include <regex>

#include <device.h>

// Initialize the Ethernet server library
// with the IP address and port you want to use

void stopDisconnectedClients();
void getTeensyMacAddr(uint8_t *mac);

IPAddress LFAST::EthernetCommsService::ip = IPAddress(192, 168, 121, 177);
EthernetServer LFAST::EthernetCommsService::server = EthernetServer(PORT);
byte LFAST::EthernetCommsService::mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
LFAST::EthernetCommsService::EthernetCommsService()
{
    bool initResult = true;
    TEST_SERIAL.println("Initializing Ethernet... ");
    getTeensyMacAddr(mac);
    // initialize the Ethernet device
    Ethernet.begin(mac, ip);
    // Ethernet.begin(mac, ip, myDns, gateway, subnet)

    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        TEST_SERIAL.println("Ethernet PHY was not found.  Sorry, can't run without hardware. :(");
        initResult = false;
    }

    // TEST_SERIAL.println("Checking Link...");
    if (Ethernet.linkStatus() == LinkOFF)
    {
        TEST_SERIAL.println("Ethernet cable is not connected.");
        initResult = false;
    }

    if (initResult)
    {
        // start listening for clients
        this->server.begin(PORT);
        TEST_SERIAL.print("Listening for connection on local IP: ");
        TEST_SERIAL.print(Ethernet.localIP());
        TEST_SERIAL.print("...");
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
        TEST_SERIAL.printf("Connection # %d Made.\r\n", connections.size() + 1);
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
