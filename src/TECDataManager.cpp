
#include "TECDataManager.h"
#include "ThermoElectricController.h"
#include "ThermoElectricGlobal.h"
#include "TimerOne.h"
#include "TECMap.h"

// Let's define some variables to hold the data to be transmitted back
volatile float m_Channel_pwr[NUM_BOARDS][CHANNELS_PER_BOARD] = {0.00};
volatile float m_Channel_seebeck[NUM_BOARDS][CHANNELS_PER_BOARD] = {0.00};
volatile float m_SeebeckStorage[CHANNELS_PER_BOARD] = {0.00};

ThermoElectricController TEC[NUM_TEC];
bool calibrated = false;
int blink = 0;

/******************
 * Begin Configure
 ******************/
// This struct is the same for every board.
const struct TEC_Channel_Config tec_ch_cfg[] =
    {
        // dir, pwm, thermistor or seebeck, min
        // Digital number for dir(ection) bit, Digital number for pwm signal, thermistor or seebeck, min
        {32, 29, 0}, // 0
        {21, 28, 0}, // 1
        {26, 25, 0}, // 2
        {20, 24, 0}, // 3
        {27, 9, 0},  // 4
        {33, 8, 0},  // 5
        {41, 5, 0},  // 6
        {19, 4, 0},  // 7
        {18, 1, 0},  // 8
        {34, 7, 0},  // 9
        {14, 36, 0}, // 10
        {40, 3, 0},  // 11
        {23, 0, 0},  // 12
        {39, 6, 0},  // 13
        {15, 37, 0}, // 14
        {22, 2, 0},  // 15
};
/*
 ******************
 * End Configure
 ******************/

/// @brief Returns a reference to the singleton instantiation of this class
///
/// The first time this is called, the static object is created and calls
/// the device's constructor. Because the static keyword is used the object
/// remains in memory, and every time the function is called after that a
/// reference to that object is returned.
///
/// @return A reference to the singleton instantiation of this class
TECDataManager &TECDataManager::getDeviceController()
{
  static TECDataManager instance;
  return instance;
}

void TECDataManager::enableControlInterrupt()
{
  Timer1.start();
}

/// @brief Interrupt for an interrupt-driven controller
///
/// If the controller is interrupt-driven, the ISR should not be part of the controller's
/// class. It should use getDeviceController() which will return a reference to the same
/// object as is used in the main.cpp code, since the device controller is a singleton.
///
void TECControl_ISR()
{
  noInterrupts();

  TECDataManager &tdm = TECDataManager::getDeviceController();
  tdm.pingCollectionStateMachine();

  // Don't want to ping the SM in the interrupt. Only want to reset it.
  interrupts();
}

void TECDataManager::pingCollectionStateMachine()
{
  // No need to collect setpoints - they are stored upon being set
  COLLECTION_STATE nextState = INIT;
  switch (currentState)
  {
  case INIT:
    nextState = WAITING;
    break;
  case WAITING:
    nextState = COLLECT_BOARD_0;
    // break;
  case COLLECT_BOARD_0:
    // getLocalSeebecks();
    if (this->isI2cController())
    {
      nextState = COLLECT_BOARD_1;
    }
    else
    {
      nextState = DONE_COLLECTING;
    }
    break;
  case COLLECT_BOARD_1:
    // getRemoteSeebecks(1);
    nextState = COLLECT_BOARD_2;
    break;
  case COLLECT_BOARD_2:
    // getRemoteSeebecks(2);
    nextState = COLLECT_BOARD_3;
    break;
  case COLLECT_BOARD_3:
    // getRemoteSeebecks(3);
    nextState = COLLECT_BOARD_4;
    break;
  case COLLECT_BOARD_4:
    // getRemoteSeebecks(4);
    nextState = DONE_COLLECTING;
    break;
  case DONE_COLLECTING:
    nextState = WAITING;
    break;
  }
  currentState = nextState;
}

void TECDataManager::processDataCommands()
{
  if (!requestList.empty())
  {
    TECDataCommand *cmdToProcess = requestList.front();
    // Process the command
    // double dutyCycle = 100.0 * cmdToProcess->value / TEC_MAX_CURRENT;
    double dutyCycle = 100.0 * cmdToProcess->value;
    // double dutyCycle = 100.0;
    if (cmdToProcess->boardNo == pTcm->getThisBoardNo())
    {
      int result = setLocalTECValue(cmdToProcess->channelNo, dutyCycle);
      cli->printfDebugMessage("Local: [b%d:c%d:pwm:%d:dir:%d] to %f%, Result=%d",
        cmdToProcess->boardNo, cmdToProcess->channelNo, 
        tec_ch_cfg[cmdToProcess->channelNo].pwmPin,tec_ch_cfg[cmdToProcess->channelNo].dirPin,
          dutyCycle, result);
    }
    else
    {
      int result = setRemoteTECValue(cmdToProcess->boardNo, cmdToProcess->channelNo, dutyCycle);
      cli->printfDebugMessage("Remote: [%d:%d] to %f%, Result=%d", cmdToProcess->boardNo, cmdToProcess->channelNo, dutyCycle, result);
    }
    delete cmdToProcess;
    requestList.pop_front();
  }
}

void TECDataManager::initialize()
{
  // Initialize Timer
  Timer1.initialize(UPDATE_PRD_US);
  Timer1.stop();
  Timer1.attachInterrupt(TECControl_ISR);
  // setup the TECs
  // delay(1000);

  cli->printDebugMessage("Setting up pins");
  for (int tec_chan = 0; tec_chan < NUM_TEC; tec_chan++)
  {
    TEC[tec_chan].begin(tec_ch_cfg[tec_chan].dirPin, tec_ch_cfg[tec_chan].pwmPin, 0.0);
  }
  // **************************
  // Initialize the SPI port  *
  // **************************
  pinMode(ADC_CHIP_SELECT_PIN, OUTPUT); // This is setting the Chip select pin to output
  initialize_SPI();
  // Finally set the power on all local channels to 0
  for (int tec_chan = 0; tec_chan < NUM_TEC; tec_chan++)
  {
    TEC[tec_chan].setDutyCycle(0);
  }

  // *************************
  // Initialize I2C
  // *************************
  bool setup_successful = false;
  if (this->isI2cController())
  {
    setup_successful &= i2c_controller_init();
  }
  else
  {
    pinMode(I2C_ID_PIN_0, INPUT_PULLUP);
    pinMode(I2C_ID_PIN_1, INPUT_PULLUP);
    i2cHardwareID_init();
    setup_successful &= i2c_peripheral_init();
  }
  currentState = INIT;
}

//***********************************************
//***********************************************
//** Functions for interfacing remote boards/TECs
//***********************************************
//***********************************************

//***********************************************
//***********************************************
//** Functions for interfacing the local TECs
//***********************************************
//***********************************************

int TECDataManager::setLocalTECValue(uint8_t tec_chan, double pwmDutyCycle)
{
  uint8_t _error = 0; // Initialize to 0 for no error... yet
  double pwmPct;

  // Need two decimal places for the pwmPct
  pwmPct = round((pwmDutyCycle)*100.0);
  // pwmPct = 50*100.0;
  // First make sure the data sent is within bounds
  if ((pwmPct < -10000) || (pwmPct > 10000))
  {
    _error = POWER_OUT_OF_RANGE;
    // Serial.println(err);
    // Can I put a return here
    return _error;
  }

  // Just set the channel to the pwmPct given
  // TEC[tec_chan].setDutyCycle(50.0);
  TEC[tec_chan].setDutyCycle(pwmPct / 100.0); // divide by 100 to get decimal places back
                                              //    Serial.println("Setting the controller board");
  m_Channel_pwr[0][tec_chan] = pwmDutyCycle;
  return _error;
} // End setTec handler

int TECDataManager::setRemoteTECValue(uint8_t board, uint8_t tec_chan, double pwmDutyCycle)
{

  uint8_t err = 0; // Initialize to 0 for no error... yet
  double pwmPct;

  // Need two decimal places for the pwmPct
  pwmPct = round((pwmDutyCycle)*100.0);

  // First make sure the data sent is within bounds
  if ((pwmPct < -10000) || (pwmPct > 10000))
  {
    err = POWER_OUT_OF_RANGE;
    // Serial.println(err);
    // Can I put a return here
    return err;
  }
  if ((board < 1) || (board > NUM_BOARDS))
  {
    err = BOARD_OUT_OF_RANGE;
    // Serial.println(err);
    return err;
  }

  char mycmd[3];
  // send i2c data to the desired board.  The peripheral boards will need to perform their own TEC[ii].set_Power()
  // this data to the other cards must include channel number and pwmPct data
  // Send all required data to the card that we are setting the power
  mycmd[0] = (tec_chan << 4) | I2C_CMD_SET_POWER;
  // Now send the pwmPct data
  // We need to break down the float variable to send it
  // Inverse operation of how we recontructed the pwmPct on this side
  short shortpwmPct = pwmPct; // Transfer float to a short data type for byte transmission
                              //    Serial.print("pwmPct: "); Serial.println(pwmPct);
                              //    Serial.print("Short pwmPct: "); Serial.println(shortpwmPct);
  mycmd[1] = shortpwmPct >> 8 & 0xff;
  mycmd[2] = shortpwmPct & 0xff;

  int addr = I2C_OFFSET + board - 1;
  Wire1.beginTransmission(addr);                    // Tell wire to start constructing a new message
  Wire1.write(mycmd, sizeof(mycmd) / sizeof(char)); // Load the data into the wire library buffer
  auto i2cError = Wire1.endTransmission();          // Wire actually sends the data now
  if (!err && !i2cError)
  {
    // Update the local copy
    m_Channel_pwr[board][tec_chan] = pwmDutyCycle;
  }
  return err;
} // End setTec handler

// void TECDataManager::getLocalSeebecks()
// {
//   for (int tec_ch = 0; tec_ch < NUM_TEC; tec_ch++)
//   {
//     auto sbVal = TEC[tec_ch].getSeebeck(tec_ch);
//     // #### Why should the first chunk of a float be the channel? - K

//     m_SeebeckStorage[tec_ch] = sbVal;
//   }
// }

// void TECDataManager::getRemoteSeebecks(uint8_t board)
// {
//   //  uint16_t seebeckvar;
//   char myseebeck[2];
//   // Here we send a command specifying data type and then request to send that data
//   // For each board, we'll go in TEC order and get each type of data
//   for (int tec_chan = 0; tec_chan < NUM_TEC; tec_chan++)
//   {
//     int addr = (board - 1) + I2C_OFFSET;
//     int cmd = (tec_chan << 4) | I2C_CMD_REPORT_SEEBECK;
//     Wire1.beginTransmission(addr);
//     Wire1.write(cmd);
//     uint8_t _error = Wire1.endTransmission(); // send the channel and data type for the following request
//     if (_error == I2C_NO_ERROR)
//     {
//       uint16_t requestedBytes = sizeof(short);
//       Wire1.requestFrom(addr, requestedBytes); // This sets how many bytes we're asking to get back
//       uint16_t receivedBytes = 0;
//       while (requestedBytes > receivedBytes)
//       {
//         auto nb = Wire1.available();
//         if (nb)
//         {
//           myseebeck[nb - 1] = Wire1.read();
//           receivedBytes += nb;
//         }
//       }
//       // while (Wire1.available())
//       // {
//       //   myseebeck[Wire1.available() - 1] = Wire1.read();
//       // }
//       m_Channel_seebeck[board][tec_chan] = ((myseebeck[1] & 0x0f) << 8) | myseebeck[0];
//       // printStuff(board, myseebeck);
//     }
//     else
//     {
//       printErrorStatus(_error, tec_chan, board);
//       if (_error == I2C_OTHER_ERROR)
//       {
//         Wire1.end();
//         delay(1000);
//         Wire1.begin();
//       }
//     }
//     // Serial.print("Full power: "); Serial.println(fullpwr);
//     delay(20); // Is this necessary?

//   } // for loop close
// }
void TECDataManager::setupPersistentFields()
{
}

void TECDataManager::printErrorStatus(uint8_t _error, uint8_t ch, uint8_t board)
{
  TEST_SERIAL.print("Channel: ");
  TEST_SERIAL.println(ch);
  TEST_SERIAL.print("Board number: ");
  TEST_SERIAL.println(board);
  TEST_SERIAL.print("Error = ");
  TEST_SERIAL.println(_error);
}

// void TECDataManager::printStuff(uint8_t board, char *myseebeck)
// {
//   for (int tec_chan = 0; tec_chan < NUM_TEC; tec_chan++)
//   {
//     TEST_SERIAL.print("Board: ");
//     TEST_SERIAL.println(board);
//     TEST_SERIAL.print("ADC Channel: ");
//     TEST_SERIAL.println(myseebeck[1] >> 4);
//     TEST_SERIAL.print("Conversion: ");
//     TEST_SERIAL.println(m_Channel_seebeck[board][tec_chan]);
//   }
// }

void TECDataManager::setAllToZero()
{
  cli->printDebugMessage("Setting all channels to zero duty");
  for (auto &tec : pTcm->cfg.tecConfigs)
  {
    if (tec->boardNo == pTcm->getThisBoardNo())
    {
      setLocalTECValue(tec->channelNo, 0.0);
    }
    else
    {
      setRemoteTECValue(tec->boardNo, tec->channelNo, 0.0);
    }
  }
}