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
/// @file TcpCommsService.cc
///

#include "../include/TcpCommsService.h"

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

void stopDisconnectedClients();


///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
LFAST::TcpCommsService::TcpCommsService(byte *ipBytes)
: IpCommsService(byte *ipBytes)
{
}

LFAST::TcpCommsService::TcpCommsService()
{
    ip = IPAddress(0, 0, 0, 0);
}

bool LFAST::TcpCommsService::initializeEnetIface(uint16_t _port)
{
    tcpServer = new EthernetServer(_port);
    IpCommsService::initializeEnetIface(_port);
    return commsServiceStatus;
}

bool LFAST::TcpCommsService::checkForNewClients()
{
    bool newClientFlag = false;
    // check for any new client connecting, and say hello (before any incoming data)
    EthernetClient newClient = tcpServer->accept();
    if (newClient)
    {
        newClientFlag = true;
        #if defined(TERMINAL_ENABLED)
        if (cli != nullptr)
            cli->printfDebugMessage("Connection # %d Made.\r\n", connections.size() + 1);
            #endif
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

