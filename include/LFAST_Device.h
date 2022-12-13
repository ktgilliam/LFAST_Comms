#pragma once

#include <TerminalInterface.h>

#define LFAST_DEVICE

class LFAST_Device
{
private:
    char DeviceName[30];
protected:
    static TerminalInterface *cli;
    virtual void setupPersistentFields() = 0;

public:
    virtual void connectTerminalInterface(TerminalInterface *_cli, const char* dev='');
    virtual void serviceCLI() {};
};