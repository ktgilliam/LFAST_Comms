#pragma once

#include <TerminalInterface.h>

#define LFAST_DEVICE

class LFAST_Device
{
private:
    bool deviceNamed;
protected:
    char DeviceName[30];
    static TerminalInterface *cli;
    virtual void setupPersistentFields() = 0;

public:
    virtual void connectTerminalInterface(TerminalInterface *_cli);
    virtual void connectTerminalInterface(TerminalInterface *_cli, const char* dev);
    virtual void serviceCLI() {};
};