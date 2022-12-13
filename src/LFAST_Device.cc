#include "LFAST_Device.h"

TerminalInterface* LFAST_Device::cli = nullptr;

void LFAST_Device::connectTerminalInterface(TerminalInterface *_cli)
{
    cli = _cli;
    setupPersistentFields();
    cli->printPersistentFieldLabels();
}