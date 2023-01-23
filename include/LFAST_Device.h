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