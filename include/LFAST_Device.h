#pragma once

#include <TerminalInterface.h>

#define LFAST_DEVICE

class LFAST_Device
{
private:
protected:
    static TerminalInterface *cli;
    virtual void setupPersistentFields() = 0;

public:
    virtual void connectTerminalInterface(TerminalInterface *_cli);
    virtual void serviceCLI() {};
};