/*******************************************************************************
Copyright 2021
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

/**
 * @file thermistorMux_global.h
 * @author Nestor Garcia (Nestor@email.arizona.edu)
 * @brief Global definitions used by all files.
 * Originally created for VCM module, modified for thermistor Mux use
 * @version (see THERMISTOR_MUX_VERSION)
 * @date 2021-03-10
 *
 * @copyright Copyright (c) 2021
 */

#include <Arduino.h>
#include <math.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>

#ifndef THERMOELECTRIC_CONTROLLER_GLOBAL_H
#define THERMOELECTRIC_CONTROLLER_GLOBAL_H

// Overall version of the VCM module
#define TEC_VERSION    "1.0 dev 1"

// Overall version of the MQTT messages.  Increment this for any change to
// the messages: added, deleted, renamed, different type, different function.
#define COMMS_VERSION  2

// Enable this to display diagnostic messages on the serial port
//#define DEBUG

//Only one of these should be defined, based on thermistor value.
#define thermistor_10K
//#define thermistor_2K

//TODO: add TEST flag maybe?

#define TEENSY_4_1

#define production_TEST
// This is where we define the number of boards
#define NUM_MODULES   5
#define MAX_BOARD_ID  (NUM_MODULES - 1)

// Display diagnostic messages on serial port if debugging is enabled
#ifdef DEBUG
#define DebugPrint( msg )       Serial.println( msg )
#define DebugPrintNoEOL( msg )  Serial.print( msg )
#else
#define DebugPrint( msg )
#define DebugPrintNoEOL( msg )
#endif

#define TEENSY_VERSION ", Teensy 4.1"

#ifdef DEBUG
  #define DEBUG_VERSION ", DEBUG"
#else
  #define DEBUG_VERSION ""
#endif

// I don't think these pins are operable anymore
#define I2C_ID_PIN_0 35
#define I2C_ID_PIN_1 38
//#define ID_PIN_2 35
//#define ID_PIN_3 34
//#define ID_PIN_4 33

// This is where we program how many TECs are on a board
#define NUMBER_OF_CHANNELS 16
const uint8_t NUM_TEC = NUMBER_OF_CHANNELS; // 12 TEC
// This is where we program how many boards are in a box
#define NUM_BOARDS NUM_MODULES


// Define some of the SPI port's pins and configuration
// Create a SPISettings instance so that we can begin and end SPI transactions
// ADS7953 doesn't make it clear if we want mode 0 or mode 1.  Mode 0 is most likely.
// SPISettings adcSettings(10000000, MSBFIRST, SPI_MODE0);
const int ADC_CHIP_SELECT_PIN = 10;            // The chip select pin for the ADC is D10
const uint16_t ADC_CMD_MODE_AUTO1 = 0x2840;    // Causes ADC to enter Auto-1 program sequence - also enable GPIO and range (?)
const uint16_t ADC_CMD_MODE_PROG = 0x8000; // ADC to enter into Auto-1 register programming mode
// const uint16_t adcChans = 0xffff;     // Choose to convert all ADC channels (a zero exclude that channel)
// const uint16_t allLow = 0x0000;       // Transmit all 0's for SPI port - utility
// const uint16_t allHigh = 0xffff;      // Transmit all 1's for SPI port - utility
const uint16_t ADC_CMD_MODE_MANUAL = 0x1040;    // Select manual mode - add channel for D10 - D07 bits in routine

#define ADC_SPI_BPS 1000000
//static float m_Channel_pwr[NUM_BOARDS][NUMBER_OF_CHANNELS] = {0.00};
//static bool m_Channel_dir[NUM_BOARDS][NUMBER_OF_CHANNELS] = {false};
//static float m_Channel_seebeck[NUM_BOARDS][NUMBER_OF_CHANNELS] = {0.00};

// Define some of the I2C port's pins and configuration
// We need to determine whether this is the controller or not
// Need to think of a way to automate that - maybe check for an ethernet connection
// If no ethernet connection, then assign as peripheral.  This might cause problems if the
// network drops or goes out.
// For now, hard code it and we'll think on it
// const boolean i2c_controller=false;
const uint8_t I2C_OFFSET = 20;
bool i2c_controller_init(void);
bool i2c_peripheral_init(void);


#define I2C_CMD_REPORT_POWER       1
// #define reportDir       2
#define I2C_CMD_REPORT_SEEBECK     8
#define I2C_CMD_SET_POWER          10

//ThermoElectricController TEC[NUM_TEC];
// Error Codes
enum ecodes 
{
  NO_BOARD_SPECIFIED = -10,
  POWER_OUT_OF_RANGE = -20,
  BOARD_OUT_OF_RANGE = -30
};
 
const int UPDATE_PRD_US = (1*1e6);

#endif
