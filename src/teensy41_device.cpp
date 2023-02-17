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
/// @file teensy41_device.cpp
///


#include "../include/teensy41_device.h"
#include "Watchdog_t4.h"
#include "CrashReport.h"

WDT_T4<WDT1> wdt;
bool wdt_ready = false;

/// @brief Prints a watchdog timer warning
void watchdogWarning()
{
    TEST_SERIAL.print("\033[31mDanger - feed the dog!\033[37m");
}

/// @brief Configure the teensy's watchdog timer
///
/// The watchdog timer resets the teensy if it gets stuck in a state which
/// prevents the loop from running. The timeout argument is optional and
/// defaults to 10 seconds. To disable the watchdog set WATCHDOG_ENABLED
/// to zero in platformio.ini
///
/// @param timeout in seconds
void configureWatchdog(uint8_t timeout)
{
    TEST_SERIAL.print("Starting watchdog");
    WDT_timings_t config;
    config.trigger = 5;  /* in seconds, 0->128 */
    config.timeout = timeout; /* in seconds, 0->128 */
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

/// @brief Prints out explanations in case a fault was detected
///
/// The ARM microcontroller on the Teensy will tell you if something in the code caused
/// the application to crash if you ask it to. The while loop is there to keep
/// the message up and give the user a chance to fix the problem before it power
/// cycles due to another crash.
///
/// @return boolean indicating whether a fault occurred.
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