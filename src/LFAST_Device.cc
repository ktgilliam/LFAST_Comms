#include "LFAST_Device.h"
#include <cstring>

TerminalInterface* LFAST_Device::cli = nullptr;

void LFAST_Device::connectTerminalInterface(TerminalInterface *_cli, const char* dev)
{
    cli = _cli;
    std::strcpy(DeviceName, dev);
    setupPersistentFields();
    cli->printPersistentFieldLabels();
}