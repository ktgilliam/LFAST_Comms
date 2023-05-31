/*

Software to control the 16-tec controller board.

Should:

Accept power commands through Ethernet or serial line (simulated)
Manage direction control.  Set current to zero before switching direction.
Report Thermistor ADC values to serial or over Ethernet port  (this would be a good place for a web server!)
*/
#include <Arduino.h>
#include <ArduinoJson.h>
#include "TECDataManager.h"
#include <teensy41_device.h>
#include <TcpCommsService.h>
#include <TerminalInterface.h>

#define BOX_A 1
#define BOX_B 2

byte myIP[]{169, 254, 84, 177};
#if THIS_BOX == BOX_A
unsigned int mPort = 4500; // Box A
#else
unsigned int mPort = 4600; // Box B
                           // byte myIP[]{169, 254, 84, 132}; // Box B
#endif

// Comms service objects and callbacks
LFAST::TcpCommsService *commsService; // This is the commsService
void handshake(unsigned int val);
void setBoxNo(unsigned int boxNo);
void boardNumber(unsigned int board);
void channelNumber(unsigned int chan_num);
void commandDutyCycle(float dc);

void sendTecData(int placeholder);
// void getTECValuesByBoard(int board);
// void getSeebeckByBoard(int board);
void resetCommandData();

TECDataManager *pTdm;
TECDataCommand *newCommandPtr = nullptr;
bool boardNoSet = false;
bool channelNoSet = false;
bool valueSet = false;

void setup()
{
  // cli = new TerminalInterface(DEVICE_CLI_LABEL, &(TEST_SERIAL), TEST_SERIAL_BAUD);

  // The DeviceController class is a singleton, (meaning only one can exist), so
  // instead of creating one with the 'new' keyword, we use its getDeviceController
  // function.
  TECDataManager &dm = TECDataManager::getDeviceController();
  pTdm = &dm;
  // pTdm->connectTerminalInterface(cli, "Device");
  pTdm->initialize();

  // *************************
  // Setup the commsService with IP and port number
  // Only for the master TEC card
  // *************************
  commsService = new LFAST::TcpCommsService(myIP);
  commsService->initializeEnetIface(mPort);
  if (!commsService->Status())
  {
    // No ethernet present -> set controller as I2C Peripheral (controller will always have ethernet)
    pTdm->peripheralMode();
  }
  else
  {
    commsService->registerMessageHandler<unsigned int>("BoxNo", setBoxNo);
    commsService->registerMessageHandler<float>("SetPoint", commandDutyCycle); // Will be changed to a current
    commsService->registerMessageHandler<unsigned int>("Board", boardNumber);
    commsService->registerMessageHandler<unsigned int>("Channel", channelNumber);
    commsService->registerMessageHandler<int>("SendAll", sendTecData);
  }

  // The ARM controller on the Teensy will tell you if something in the code caused
  // The application to crash if you ask it to. The while loop is there to keep
  // the message up and give the user a chance to fix the problem before it power
  // cycles due to another crash.
  bool crashOccurred = checkForCrashReport();
  while (crashOccurred)
  {
    ;
  }

  
}

void loop()
{
#if WATCHDOG_ENABLED
  feedWatchDog();
#endif

  if (pTdm->isI2cController())
  {
    commsService->checkForNewClients();
    if (commsService->checkForNewClientData())
    {
      commsService->processClientData("TECCommand");
    }
    commsService->stopDisconnectedClients();
    pTdm->processDataCommands();
  }

}

/// @brief Handshake function to confirm connection
/// @param val The value that was sent with the handshake key
void handshake(unsigned int val)
{
  if (val == 0xDEAD)
  {
    LFAST::CommsMessage newMsg;
    newMsg.addKeyValuePair<unsigned int>("Handshake", 0xBEEF);
    commsService->sendMessage(newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
    // cli->printDebugMessage("Connected to client, starting control ISR.");
    pTdm->enableControlInterrupt();
  }
  return;
}

void setBoxNo(unsigned int boxNo)
{
  pTdm->setBoxNo(boxNo);
}

void boardNumber(unsigned int board)
{
  if (newCommandPtr != nullptr)
  {
    // Something went wrong
    delete newCommandPtr;
    resetCommandData();
  }
  else
  {
    newCommandPtr = new TECDataCommand();
    newCommandPtr->boardNo = board;
    boardNoSet = true;
  }
  return;
}

void channelNumber(unsigned int channel)
{
  if (newCommandPtr != nullptr)
  {
    newCommandPtr->channelNo = channel;
    channelNoSet = true;
  }
  return;
}
void commandDutyCycle(float dc)
{
  if (newCommandPtr != nullptr)
  {
    newCommandPtr->type = DUTY_COMMAND;
    valueSet = true;
    newCommandPtr->value = dc;
    if(boardNoSet && channelNoSet && valueSet)
    {
      pTdm->addTecDataCommand(newCommandPtr);
      resetCommandData();
    }
  }
  return;
}

void resetCommandData()
{
  newCommandPtr = nullptr;
  boardNoSet = false;
  channelNoSet = false;
  valueSet = false;
}
void sendTecData(int placeholder)
{
}