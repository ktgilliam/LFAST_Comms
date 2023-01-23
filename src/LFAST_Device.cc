#include "LFAST_Device.h"
#include <cstring>
#include <cstdio>
// #include "teensy41_device.h"
#if defined(TERMINAL_ENABLED)
TerminalInterface *LFAST_Device::cli = nullptr;

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

void LFAST_Device::connectTerminalInterface(TerminalInterface *_cli, const char *dev)
{
    std::strcpy(DeviceName, dev);
    deviceNamed = true;
    connectTerminalInterface(_cli);
}
#endif