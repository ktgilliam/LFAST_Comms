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
/// @file UdpCommsService.h
///
/// The LFAST Comms library (of which UdpCommsService is a component)
/// works by associating JSON key-value pairs to function pointers.
/// When a key is received, the library checks to see if a function 
/// pointer has been registered for it. If it has, it calls that function
/// and passes the value from the key-value pair as an argument.
/// This template defines and registers two such callbacks in this file.

#pragma once

#include <cstdint>
#include <list>


#ifdef TEENSYDUINO
#include <NativeEthernet.h>
#else
#include <SPI.h>
#include <Ethernet.h>
#endif

#include <EthernetUdp.h>
#include "CommService.h"


#define MAX_CLIENTS 4
namespace LFAST
{
    class UdpCommsService : public CommsService
    {
    protected:
    static bool hardwareConfigurationDone;
        void getTeensyMacAddr(uint8_t *mac);
        static byte mac[6];
        IPAddress ip;
        EthernetUDP *udp;

    public:
        UdpCommsService();
        UdpCommsService(byte *);

        bool initializeEnetIface(uint16_t);

        bool Status() { return this->commsServiceStatus; };
    };
}