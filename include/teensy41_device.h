 /// 
 ///  @ Author: Kevin Gilliam
 ///  @ Create Time: 2022-09-07 08:34:54
 ///  @ Modified by: Kevin Gilliam
 ///  @ Modified time: 2022-09-08 12:39:33
 ///  @ Description:
 ///

#pragma once

#include "macro.h"
#include <Arduino.h>

#if defined(ARDUINO_TEENSY41)
#define NUM_SERIAL_DEVICES 8
#else
#define NUM_SERIAL_DEVICES 7
#endif


#define SERIAL_CH(N) CONCAT(Serial, N)

#if defined(TEST_SERIAL_NO)
#define ENABLE_TERMINAL 1
#else
#define ENABLE_TERMINAL 0
#endif

#if defined(ENABLE_TERMINAL) && !defined(TEST_SERIAL_NO)
#warning "Terminal enabled but test serial not defined."
#warning "Define TEST_SERIAL_NO in build flags."
#warning "Defaulting to Hardware serial #2 at 230400 bps."
#define TEST_SERIAL_NO 2
#endif

#if defined(ENABLE_TERMINAL) && !defined(TEST_SERIAL_BAUD)
#warning "Terminal enabled but test serial not defined."
#warning "Define TEST_SERIAL_BAUD in build flags."
#warning "Defaulting to Hardware serial #2 at 230400 bps."
#define TEST_SERIAL_BAUD (uint32_t)230400
#endif

#define TEST_SERIAL SERIAL_CH(TEST_SERIAL_NO)

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


#define MODBUS_SERIAL_NO 1
#define MODBUS_SERIAL SERIAL_CH(MODBUS_SERIAL_NO)
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

#define MODBUS_RTS_PIN 6


// #define MODE_PIN         31
#define LED_PIN          13

// #define CAN3_PIN 35
// #define CAN1_PIN 34


#define DEBUG_PIN_1 32

#define TOGGLE_DEBUG_PIN() digitalWrite(DEBUG_PIN_1, !digitalRead(DEBUG_PIN_1));

#define TOGGLE_LED_PIN() digitalWrite(LED_PIN, !digitalRead(LED_PIN));

