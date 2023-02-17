#include "../include/teensy41_device.h"
#include "Watchdog_t4.h"
#include "CrashReport.h"

WDT_T4<WDT1> wdt;
bool wdt_ready = false;

void watchdogWarning()
{
    if (cli != nullptr)
    {
        cli->printDebugMessage("Danger - feed the dog!", LFAST::WARNING_MESSAGE);
    }
}

void configureWatchdog()
{
    if (cli != nullptr)
    {
        cli->printDebugMessage("Starting watchdog");
    }
    WDT_timings_t config;
    config.trigger = 5;  /* in seconds, 0->128 */
    config.timeout = 10; /* in seconds, 0->128 */
    config.callback = watchdogWarning;
    config.pin = LED_PIN;
    pinMode(LED_PIN, OUTPUT);
#if WATCHDOG_ENABLED
    wdt_ready = true;
    wdt.begin(config);
#endif
}

void feedWatchDog()
{
    if (wdt_ready)
        wdt.feed();
}

bool checkForCrashReport()
{
    bool thereWasACrash = false;
    if (CrashReport)
    {
        CrashReport.printTo(TEST_SERIAL);
        TEST_SERIAL.print("\nPower cycle to clear this error.");
        thereWasACrash = true;
    }
    return thereWasACrash;
}