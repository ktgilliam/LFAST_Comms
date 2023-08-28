/*******************************************************************************
Copyright 2022
Steward Observatory Engineering & Channelhnical Services, University of Arizona

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
 * @file ThermoElectricTWI.cpp
 * @author Stephen Kaye (kaye2@arizona.edu)
 * @brief Implements i2c specific functions to talk to peripheral boards or
 * to receive information from the controller board.
 * Initial instance.
 * @version (initial)
 * @date 2022-11-22
 *
 * @copyright Copyright (c) 2022
 */

// #include "ThermoElectricNetwork.h"
#include "ThermoElectricGlobal.h"
#include "ThermoElectricController.h"
#include "ThermoElectricTWI.h"
#include <Wire.h>

extern float m_SeebeckStorage[CHANNELS_PER_BOARD];
extern ThermoElectricController TEC[NUM_TEC];

int8_t cmdCode, pwmPctHighByte, pwmPctLowByte;
void reportSetPointHandler(uint8_t chan);
void reportSeebeckHandler(uint8_t chan);
/**
 * @brief Initializes the i2c network, if a controller, it will parse out information received
 * from the ethernet network.  If a peripheral, it will set up a callback function for when the board
 * receives information.
 *
 * @return true on success
 * @return false if there's some failure
 */
bool i2c_controller_init(void)
{
    // The question is to make this general so all Teensy's get the same code
    // or do we have separate code for controllers and peripherals?
    // Probably should do a compiling switch
    // If we are the controller, join as a controller.  No parameter for the Wire1.begin command
    Wire1.begin(); // No return value for begin()
    // Wire1.setClock(400000L); // slow the clock to 10 kHz
    // There's no return values for begin, so can't do a proper error check
    return true;
}
/**
 * @brief Initializes the i2c network, if a controller, it will parse out information received
 * from the ethernet network.  If a peripheral, it will set up a callback function for when the board
 * receives information.
 *
 * @return true on success
 * @return false if there's some failure
 */
bool i2c_peripheral_init(void)
{
    // We are a peripheral, join with an address as the parameter
    // Use the hardware_id to create the address
    // Add 20 to hardware_id since lower i2c addresses may be reserved by the Teensy
    // Serial.println("i2c configured as a peripheral");
    // Serial.println(get_hardware_id() + I2C_OFFSET);
    Wire1.begin(get_hardware_id() + I2C_OFFSET); // No return value for begin()
    // Wire1.setClock(10000L); // slow the clock to 10 kHz
    //  Install some callbacks for the peripheral device
    Wire1.onRequest(requestEventHandler);
    Wire1.onReceive(receiveEventHandler);
    // There's no return values for begin, so can't do a proper error check
    return true;
}
/**
 * @brief Callback for a receiveEvent.  This will get the data the controller is sending.  Data will
 * determine what kind of data is being requested.  This will be passed on to the requestEvent callback
 *
 *
 * @return No return value.
 *
 */
void receiveEventHandler(int bytesRecv)
{
    byte recvData[bytesRecv];
    int tec_chan;

    for (int ii = 0; ii < bytesRecv; ii++)
    {
        recvData[bytesRecv - ii - 1] = Wire1.read();
    }
    cmdCode = recvData[bytesRecv - 1];

    if (bytesRecv > 1) // ##### Only happens if sending a new TEC setpoint, but we should parse the cmd code instead - K
    {
        pwmPctHighByte = recvData[bytesRecv - 2];
        pwmPctLowByte = recvData[bytesRecv - 3];
        tec_chan = (cmdCode >> 4) & 0x0f; // Get MSNibble
                                          //        Serial.print("High Byte: "); Serial.println((pwmPctHighByte << 8));
                                          //        Serial.print("Low Byte: "); Serial.println(pwmPctLowByte & 0xff);
                                          //        Serial.print("Setting: "); Serial.println((((pwmPctHighByte << 8) | (pwmPctLowByte & 0xff))/100.0));
        TEC[tec_chan].setDutyCycle((((pwmPctHighByte << 8) | (pwmPctLowByte & 0xff)) / 100.0));
    }
    return;
}

/**
 * @brief Callback for a requestEvent.  This will send data that was requested.  The command
 * specifying the data was received in the receiveEvenHandler and now is passed to the requestEventHandler
 *
 *
 * @return Nothing.  The data will be received by the controller over the i2c bus
 * @return
 */
void requestEventHandler(void)
{
    int dataReq, chan;

    // This is where we send back data according to the cmdCode
    // cmdCode Most Sig Nibble channel
    // cmdCode Least Sig Nibble command (whcih data is requested)
    // cmdCode = 5 send the duty cycle settings
    // cmdCode = 20 send the Seebeck data

    dataReq = cmdCode & 0x0F;     // Get LSNibble
    chan = (cmdCode >> 4) & 0x0f; // Get MSNibble
    // Serial.print("datareq: "); Serial.println(dataReq);
    // Serial.print("channel: "); Serial.println(chan);
    // Serial.print("cmdCode: "); Serial.println(cmdCode);
    switch (dataReq)
    {
    case I2C_CMD_REPORT_POWER:
        reportSetPointHandler(chan);
        break;
    case I2C_CMD_REPORT_SEEBECK:
        reportSeebeckHandler(chan);
        break;
    default:
        Serial.println("In request event for i2c, but no match");
        break;
    }

    return;
}

void reportSetPointHandler(uint8_t chan)
{
    byte data_by_byte[2];
    short data;
    bool dir;
    /* code */
    data = round(TEC[chan].getDutyCycle() * 100.0); // This gets the power for the requested channel, now we need to send it
                                                //        Serial.print("Data: "); Serial.println(data);
    dir = TEC[chan].getDirection();
    data *= dir * -1;
    if (data < 0)
    {
        data_by_byte[0] = (data >> 8) & 0xff; // This is a negative flag to set upper bits to 1 for a negative number when bytes are reassembled at the controller
    }
    else
    {
        data_by_byte[0] = (data >> 8) & 0xff; // This keeps the upper bits set to 0 when bytes are reassembled at the controller
    }
    data_by_byte[1] = (data & 0xff);
    //        Serial.print("High Byte: "); Serial.println(data_by_byte[0]);
    //        Serial.print("Low byte: "); Serial.println(data_by_byte[1]);
    for (int ii = 0; ii < 2; ii++)
    {
        Wire1.write(data_by_byte[ii]); // We just write since this is a peripheral and the controller requested data
    }
}

void reportSeebeckHandler(uint8_t chan)
{
    short data;
    /* getSeebeck returns a 16 bit value, top 4 bits is ADC channel number and the remaining 12 bits is the ADC value */
    //        Serial.print("reportSeebeck: "); Serial.println(chan);
    data = m_SeebeckStorage[chan]; // TEC[chan].getSeebeck(chan);
                                   //        Serial.print("Seebeck data: "); Serial.println((uint16_t)m_SeebeckStorage[chan]&0x0fff);
    byte b1 = (data >> 8) & 0xff;
    byte b2 = data & 0xff;
    Wire1.write(b1);
    Wire1.write(b2);
}
