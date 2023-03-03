/*******************************************************************************
Copyright 2022
Steward Observatory Engineering & Technical Services, University of Arizona
This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*******************************************************************************/
///
/// @author Kevin Gilliam
/// @date February 16th, 2023
/// @file UdpCommsService.cc
///

#include "../include/UdpCommsService.h"

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
#include <stdlib.h> /* atoi */

#include <vector>
#include <iterator>
// #include <regex>

// Initialize the Ethernet server library
// with the IP address and port you want to use

void getTeensyMacAddr(uint8_t *mac);
IPAddress parseIpAddress(byte *bytes);

bool LFAST::UdpCommsService::hardwareConfigurationDone = false;
// const IPAddress defaultIp = IPAddress(192, 168, 121, 177);
// const uint16_t defaultPort = DEFAULT_PORT;

byte LFAST::UdpCommsService::mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
LFAST::UdpCommsService::UdpCommsService(byte *ipBytes)
{
    ip = IPAddress(ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]);
}

LFAST::UdpCommsService::UdpCommsService()
{
    ip = IPAddress(0, 0, 0, 0);
}

bool LFAST::UdpCommsService::initializeEnetIface(uint16_t _port)
{
    bool initResult = true;
    tcpServer = new EthernetServer(_port);

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
        hardwareConfigurationDone = true;
    }
    if (initResult)
        commsServiceStatus = true;
    return commsServiceStatus;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// LOCAL/PRIVATE FUNCTIONS ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

void LFAST::UdpCommsService::getTeensyMacAddr(uint8_t *mac)
{
    for (uint8_t by = 0; by < 2; by++)
        mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
    for (uint8_t by = 0; by < 4; by++)
        mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
}