/*

Software to control the 16-tec controller board.

Should:

Accept power commands through Ethernet or serial line (simulated)
Manage direction control.  Set current to zero before switching direction.
Report Thermistor ADC values to serial or over Ethernet port  (this would be a good place for a web server!)
*/
#include <Arduino.h>
#include <cinttypes>
#include <teensy41_device.h>
#include <TcpCommsService.h>
#include <TerminalInterface.h>
#include "TECDataManager.h"
#include "TECConfigManager.h"
#include "teensy41_device.h"

// #define BOX_A 1
// #define BOX_B 2

// uint8_t myIP[]{169, 254, 84, 177};
// #if THIS_BOX == BOX_A
// unsigned int mPort = 4500; // Box A
// #else
// unsigned int mPort = 4600; // Box B
//                            // byte myIP[]{169, 254, 84, 132}; // Box B
// #endif
TerminalInterface *cli;
// Comms service objects and callbacks
LFAST::TcpCommsService *commsService; // This is the commsService
void handshake(unsigned int val);
void setTecNo(unsigned int tecNo);
void boardNumber(unsigned int board);
void channelNumber(unsigned int chan_num);
void commandTecDutyCycle(float dc);
void allToZero(int placeholder);
void sendTecData(int placeholder);
// void getTECValuesByBoard(int board);
// void getSeebeckByBoard(int board);

TECDataManager *pTdm;
TECDataCommand *newCommandPtr = nullptr;

TECConfigManager tcm;
TECConfigManager *pTcm = &tcm;

void setup()
{
  // delay(3000);
#if HARDWARE_SERIAL == 1
  cli = new TerminalInterface(DEVICE_CLI_LABEL, &(TEST_SERIAL), TEST_SERIAL_BAUD);
#else
  cli = new TerminalInterface(DEVICE_CLI_LABEL, &(TEST_SERIAL));
#endif
  TECDataManager &tdm = TECDataManager::getDeviceController();
  pTdm = &tdm;
  pTdm->connectTerminalInterface(cli, DEVICE_CLI_LABEL);
  pTdm->initialize();

  bool configPresent = pTcm->parseConfiguration("config.json");
  // bool configPresent = true;
  if (configPresent)
  {
    pTdm->controllerMode();
    pTdm->connectConfigManager(pTcm);
    // cli->printDebugMessage("1");
    // *************************
    // Setup the commsService with IP and port number
    // Only for the master TEC card
    // *************************
    // commsService = new LFAST::TcpCommsService(myIP);
    commsService = new LFAST::TcpCommsService(tcm.cfg.ip);
    commsService->connectTerminalInterface(cli);
    // commsService->initializeEnetIface(mPort);
    commsService->initializeEnetIface(tcm.cfg.port);
    cli->printDebugMessage("Config IS present.");
    // TEST_SERIAL.printf("%d.%d.%d.%d:%d\r\n", tcm.cfg.ip[0], tcm.cfg.ip[1], tcm.cfg.ip[2], tcm.cfg.ip[3], tcm.cfg.port);
    if (commsService->Status())
    {
      //
      commsService->registerMessageHandler<unsigned int>("Handshake", handshake);
      commsService->registerMessageHandler<unsigned int>("TECNo", setTecNo);
      commsService->registerMessageHandler<float>("SetDuty", commandTecDutyCycle); // Will be changed to a current

      // commsService->registerMessageHandler<unsigned int>("Board", boardNumber);
      // commsService->registerMessageHandler<unsigned int>("Channel", channelNumber);
      commsService->registerMessageHandler<int>("SendAll", sendTecData);
      commsService->registerMessageHandler<int>("AllToZero", allToZero);
    }
  }
  else
  {
    cli->printDebugMessage("Config is NOT present.");
    pTdm->peripheralMode();
  }
#if WATCHDOG_ENABLED
  // The watchdog timer resets the teensy if it gets stuck in a state which
  // prevents the loop from running
  configureWatchdog();
#endif
  // The ARM controller on the Teensy will tell you if something in the code caused
  // The application to crash if you ask it to. The while loop is there to keep
  // the message up and give the user a chance to fix the problem before it power
  // cycles due to another crash.
  bool crashOccurred = checkForCrashReport();
  // Note: C:\Users\kevin\AppData\Local\Arduino15\packages\arduino\tools\avr-gcc\7.3.0-atmel3.6.1-arduino7\bin\avr-addr2line.exe
  while (crashOccurred)
  {
    ;
  }
  cli->printDebugMessage("Setup Done.", LFAST::INFO_MESSAGE);
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
  else
  {
    
  }
  // Serial.println("inside loop.");
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
    cli->printDebugMessage("Connected to client, starting control ISR.");
    pTdm->enableControlInterrupt();
  }
  return;
}

void setTecNo(unsigned int tecNo)
{
  // pTdm->setTecNo(tecNo);
  cli->printfDebugMessage("New TEC Command: %d", tecNo);
  auto tecCfg = pTcm->getTecConfig(tecNo);
  if (tecCfg != nullptr)
  {
    newCommandPtr = new TECDataCommand(tecCfg->boardNo, tecCfg->channelNo);
  }
}

void commandTecDutyCycle(float currAmps)
{
  // If the box number didn't match, this should still be nullptr
  if (newCommandPtr != nullptr)
  {
    newCommandPtr->type = CURRENT_COMMAND;
    newCommandPtr->value = currAmps;
    pTdm->addTecDataCommand(newCommandPtr);
    newCommandPtr = nullptr;
  }
  return;
}


void allToZero(int placeholder)
{
  cli->printDebugMessage("Inside allToZero");
  pTdm->setAllToZero();
}

void sendTecData(int placeholder)
{
  cli->printDebugMessage("Inside sendTecData");
  LFAST::CommsMessage *newMsg = new LFAST::CommsMessage();
  bool result = false;
  unsigned int sentCfgs = 0;
  int sentMessages = 0;
  for (auto &tec : pTcm->cfg.tecConfigs)
  {
    result = newMsg->startNewArrayObject("tecConfigList");
    if (!result)
    {
      commsService->sendMessage(*newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
      sentMessages++;
      delete newMsg;
      newMsg = new LFAST::CommsMessage();
      result = newMsg->startNewArrayObject("tecConfigList");
    }
    newMsg->addKeyValuePairToArrayItemObject<unsigned int>("ID", tec->tecNo);
    newMsg->addKeyValuePairToArrayItemObject<unsigned int>("BRD", tec->boardNo);
    newMsg->addKeyValuePairToArrayItemObject<unsigned int>("CHN", tec->channelNo);
    // FIXME::!!! There is a problem where if the buffer is full it will just leave one of these out. It needs to be fixed properly in CommService!!!!!
    sentCfgs++;
  }
  newMsg->addKeyValuePair("SentConfigs", sentCfgs);
  commsService->sendMessage(*newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
  sentMessages++;
  cli->printfDebugMessage("Sent %d configs over %d messages.", sentCfgs, sentMessages);

  delete newMsg;
}