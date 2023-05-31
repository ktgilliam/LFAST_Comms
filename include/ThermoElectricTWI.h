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


void receiveEventHandler(int bytesRecv);
void requestEventHandler(void);
