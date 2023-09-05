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
/// @file teensy41_device.h
///

#pragma once

#include "macro.h"
#include <Arduino.h>

#ifdef DEVICE_LABEL
#define DEVICE_CLI_LABEL STR(DEVICE_LABEL)
#else
#define DEVICE_CLI_LABEL "LFAST_DEVICE"
#endif

#define SERIAL_CH(N) CONCAT(Serial, N)

#if defined(ARDUINO_TEENSY41)
#define NUM_SERIAL_DEVICES 8
#else
#define NUM_SERIAL_DEVICES 7
#endif

// #if !defined(TERMINAL_ENABLED) && !defined(TERMINAL_DISABLED)
// #if defined(TEST_SERIAL_NO)
// // #define TERMINAL_ENABLED
// // #else
// // #define TERMINAL_DISABLED
// #endif
// #endif

// #if defined(TERMINAL_ENABLED) && !defined(TEST_SERIAL_NO)
// #warning "Terminal enabled but test serial not defined."
// #warning "Define TEST_SERIAL_NO in build flags."
// #warning "Defaulting to Hardware serial #2 at 230400 bps."
// #define TEST_SERIAL_NO 2
// #endif

// #if defined(TERMINAL_ENABLED) && !defined(TEST_SERIAL_BAUD)
// #warning "Terminal enabled but test serial not defined."
// #warning "Define TEST_SERIAL_BAUD in build flags."
// #warning "Defaulting to Hardware serial #2 at 230400 bps."
// #define TEST_SERIAL_BAUD (uint32_t)230400
// #endif



#if defined(TEST_SERIAL_NO)
    #define TEST_SERIAL_TYPE HardwareSerial
    #if TEST_SERIAL_NO==1
        #define TEST_SERIAL_RX_PIN 0
        #define TEST_SERIAL_TX_PIN 1
        // #define TEST_SERIAL_RX_PIN 52
        // #define TEST_SERIAL_TX_PIN 53
    #elif  TEST_SERIAL_NO==2
        #define TEST_SERIAL_RX_PIN 7
        #define TEST_SERIAL_TX_PIN 8
    #elif  TEST_SERIAL_NO==3
        #define TEST_SERIAL_RX_PIN 15
        #define TEST_SERIAL_TX_PIN 14
    #elif  TEST_SERIAL_NO==4
        #define TEST_SERIAL_RX_PIN 16
        #define TEST_SERIAL_TX_PIN 17
    #elif  TEST_SERIAL_NO==5
        #define TEST_SERIAL_RX_PIN 21
        #define TEST_SERIAL_TX_PIN 20
        // #define TEST_SERIAL_RX_PIN 46
        // #define TEST_SERIAL_TX_PIN 47
    #elif  TEST_SERIAL_NO==6
        #define TEST_SERIAL_RX_PIN 25
        #define TEST_SERIAL_TX_PIN 24
    #elif  TEST_SERIAL_NO==7
        #define TEST_SERIAL_RX_PIN 28
        #define TEST_SERIAL_TX_PIN 29
    #elif  TEST_SERIAL_NO==8
        #define TEST_SERIAL_RX_PIN 34
        // #define TEST_SERIAL_RX_PIN 48
        #define TEST_SERIAL_TX_PIN 35
    #endif

    #define TEST_SERIAL SERIAL_CH(TEST_SERIAL_NO)
#else
    #define TEST_SERIAL Serial
    #define TEST_SERIAL_TYPE usb_serial_class
#endif

#ifndef TEST_SERIAL_BAUD
#define TEST_SERIAL_BAUD 9600
#endif

#define MODBUS_SERIAL_NO 1

#define MODBUS_SERIAL_BAUD 115200

#if MODBUS_SERIAL_NO==1
    #define MODBUS_SERIAL_RX_PIN 0
    #define MODBUS_SERIAL_TX_PIN 1
    // #define MODBUS_SERIAL_RX_PIN 52
    // #define MODBUS_SERIAL_TX_PIN 53
#elif  MODBUS_SERIAL_NO==2
    #define MODBUS_SERIAL_RX_PIN 7
    #define MODBUS_SERIAL_TX_PIN 8
#elif  MODBUS_SERIAL_NO==3
    #define MODBUS_SERIAL_RX_PIN 15
    #define MODBUS_SERIAL_TX_PIN 14
#elif  MODBUS_SERIAL_NO==4
    #define MODBUS_SERIAL_RX_PIN 16
    #define MODBUS_SERIAL_TX_PIN 17
#elif  MODBUS_SERIAL_NO==5
    #define MODBUS_SERIAL_RX_PIN 21
    #define MODBUS_SERIAL_TX_PIN 20
    // #define MODBUS_SERIAL_RX_PIN 46
    // #define MODBUS_SERIAL_TX_PIN 47
#elif  MODBUS_SERIAL_NO==6
    #define MODBUS_SERIAL_RX_PIN 25
    #define MODBUS_SERIAL_TX_PIN 24
#elif  MODBUS_SERIAL_NO==7
    #define MODBUS_SERIAL_RX_PIN 28
    #define MODBUS_SERIAL_TX_PIN 29
#elif  MODBUS_SERIAL_NO==8
    #define MODBUS_SERIAL_RX_PIN 34
    // #define MODBUS_SERIAL_RX_PIN 48
    #define MODBUS_SERIAL_TX_PIN 35
#endif

#define MODBUS_SERIAL SERIAL_CH(MODBUS_SERIAL_NO)

#define MODBUS_RTS_PIN 6


// #define MODE_PIN         31
#define LED_PIN          13

// #define CAN3_PIN 35
// #define CAN1_PIN 34


#define DEBUG_PIN_1 32

#define TOGGLE_DEBUG_PIN() digitalWrite(DEBUG_PIN_1, !digitalRead(DEBUG_PIN_1));
#define SET_DEBUG_PIN() digitalWrite(DEBUG_PIN_1, 1);
#define CLEAR_DEBUG_PIN() digitalWrite(DEBUG_PIN_1, 0);
#define TOGGLE_LED_PIN() digitalWrite(LED_PIN, !digitalRead(LED_PIN));


#define PRINT_GOT_HERE() TEST_SERIAL.printf("@ %s: %d\r\n", __FILE__, __LINE__);delay(100);

void configureWatchdog(uint8_t timeout=10);
void watchdogWarning();
void feedWatchDog();

bool checkForCrashReport();