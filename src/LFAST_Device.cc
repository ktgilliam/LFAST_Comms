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
/// @file LFAST_Device.cc
///

#include "LFAST_Device.h"
#include <cstring>
#include <cstdio>
// #include "teensy41_device.h"

TerminalInterface *LFAST_Device::cli = nullptr;

/// @brief Assigns a pointer to the terminal object
/// @param _cli Pointer to a TerminalInterface object
void LFAST_Device::connectTerminalInterface(TerminalInterface *_cli)
{
    static uint8_t deviceCount = 0;

    if (_cli == nullptr)
    {
        // TEST_SERIAL.println("null!");
        return;
    }

    deviceCount++;
    if (!deviceNamed)
    {
        std::sprintf(DeviceName, "Device_%d", deviceCount);
        deviceNamed = true;
    }
    cli = _cli;
    cli->registerDevice(DeviceName);
    setupPersistentFields();
    // cli->printDebugInfo();
}

/// @brief Assigns a pointer to the terminal object
/// @param _cli Pointer to a TerminalInterface object
void LFAST_Device::connectTerminalInterface(TerminalInterface *_cli, const char *dev)
{
    std::strcpy(DeviceName, dev);
    deviceNamed = true;
    connectTerminalInterface(_cli);
}