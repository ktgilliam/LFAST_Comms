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
/// @file LFAST_Device.h
///

#pragma once

#include <teensy41_device.h>

#if defined(TERMINAL_ENABLED)
#include <TerminalInterface.h>
#endif

#define LFAST_DEVICE

class LFAST_Device
{
private:
    bool deviceNamed;

protected:
    char DeviceName[30];
#if defined(TERMINAL_ENABLED)
    static TerminalInterface *cli;
    virtual void setupPersistentFields() = 0;
#endif

public:
#if defined(TERMINAL_ENABLED)
    virtual void connectTerminalInterface(TerminalInterface *_cli);
    virtual void connectTerminalInterface(TerminalInterface *_cli, const char *dev);
    virtual void serviceCLI(){};
#endif

};