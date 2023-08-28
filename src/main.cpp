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
// void boardNumber(unsigned int board);
// void channelNumber(unsigned int chan_num);
void commandTecDutyCycle(float dc);
void allToZero(int placeholder);
void sendTecConfigData(int placeholder);
// void getTECValuesByBoard(int board);
// void getSeebeckByBoard(int board);

TECDataManager *pTdm;
TECDataCommand *newCommandPtr = nullptr;

TECConfigManager tcm;
TECConfigManager *pTcm = &tcm;

void setup()
{
  // delay(3000);
  cli = new TerminalInterface(DEVICE_CLI_LABEL, &(TEST_SERIAL), TEST_SERIAL_BAUD);
#if WATCHDOG_ENABLED
  // The watchdog timer resets the teensy if it gets stuck in a state which
  // prevents the loop from running
  configureWatchdog();
  cli->printDebugMessage("Starting watchdog");
#endif
  TECDataManager &tdm = TECDataManager::getDeviceController();
  pTdm = &tdm;
  pTdm->connectTerminalInterface(cli, DEVICE_CLI_LABEL);
  pTdm->initialize();

  bool configPresent = pTcm->parseConfiguration("config.json");
  // bool configPresent = true;
  if (configPresent)
  {
    cli->printDebugMessage("Config IS present.");
    pTdm->controllerMode();
    pTdm->connectConfigManager(pTcm);
    // cli->printDebugMessage("1");
    // *************************
    // Setup the commsService with IP and port number
    // Only for the master TEC card
    // *************************
    // commsService = new LFAST::TcpCommsService(myIP);
    cli->printDebugMessage("Setting up TCP/IP.");
    commsService = new LFAST::TcpCommsService(tcm.cfg.ip);
    commsService->connectTerminalInterface(cli);
    // commsService->initializeEnetIface(mPort);
    commsService->initializeEnetIface(tcm.cfg.port);
    // TEST_SERIAL.printf("%d.%d.%d.%d:%d\r\n", tcm.cfg.ip[0], tcm.cfg.ip[1], tcm.cfg.ip[2], tcm.cfg.ip[3], tcm.cfg.port);
    if (commsService->Status())
    {
      //
      cli->printDebugMessage("Setting up handlers");
      commsService->registerMessageHandler<unsigned int>("Handshake", handshake);
      commsService->registerMessageHandler<unsigned int>("TECNo", setTecNo);
      commsService->registerMessageHandler<float>("SetDuty", commandTecDutyCycle); // Will be changed to a current

      // commsService->registerMessageHandler<unsigned int>("Board", boardNumber);
      // commsService->registerMessageHandler<unsigned int>("Channel", channelNumber);
      commsService->registerMessageHandler<int>("SendAll", sendTecConfigData);
      commsService->registerMessageHandler<int>("AllToZero", allToZero);
    }
    pTdm->setAllToZero();
  }
  else
  {
    cli->printDebugMessage("Config is NOT present.");
    pTdm->peripheralMode();
  }

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

  // auto testCmd = new TECDataCommand(0, 0, -0.4);
  // pTdm->addTecDataCommand(testCmd);
  // pTdm->processDataCommands();
  // while (1)
  // {
  //   ;
  // }
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
  auto tecCfg = pTcm->getTecConfig(tecNo);
  cli->printfDebugMessage("TEC Command: %d [%d:%d]", tecNo, tecCfg->boardNo, tecCfg->channelNo);
  if (tecCfg != nullptr)
  {
    newCommandPtr = new TECDataCommand(tecCfg->boardNo, tecCfg->channelNo);
  }
}

void commandTecDutyCycle(float duty)
{
  // cli->printfDebugMessage("Duty Cycle: %4.6f", duty);
  // If the box number didn't match, this should still be nullptr
  if (newCommandPtr != nullptr)
  {
    newCommandPtr->type = DUTY_REQUEST;
    newCommandPtr->value = duty;
    pTdm->addTecDataCommand(newCommandPtr);
    newCommandPtr = nullptr;
  }
  else
  {
    cli->printDebugMessage("Command with no TEC number.", LFAST::ERROR_MESSAGE);
  }
  return;
}

void allToZero(int placeholder)
{
  cli->printDebugMessage("Inside allToZero");
  pTdm->setAllToZero();
}

void sendTecConfigData(int placeholder)
{
  cli->printDebugMessage("Inside sendTecConfigData");
  LFAST::CommsMessage *newMsg = new LFAST::CommsMessage();
  bool result = false;
  unsigned int sentCfgs = 0;
  int sentMessages = 0;

  // newMsg->addKeyValuePair<unsigned int>("num_tecs",  pTcm->cfg.tecConfigs.size());
  // commsService->sendMessage(*newMsg, LFAST::CommsService::ACTIVE_CONNECTION);

  for (auto &tec : pTcm->cfg.tecConfigs)
  {
    result = newMsg->startNewArrayObjectItem("tecConfigList");
    if (!result)
    {
      // If it's full, send it and start a new one.
      commsService->sendMessage(*newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
      sentMessages++;
      delete newMsg;
      newMsg = new LFAST::CommsMessage();
      result = newMsg->startNewArrayObjectItem("tecConfigList");
    }
    newMsg->addKeyValuePairToArrayObjectItem<unsigned int>("ID", tec->tecNo);
    newMsg->addKeyValuePairToArrayObjectItem<unsigned int>("BRD", tec->boardNo);
    newMsg->addKeyValuePairToArrayObjectItem<unsigned int>("CHN", tec->channelNo);
    // FIXME::!!! There is a problem where if the buffer is full it will just leave one of these out. It needs to be fixed properly in CommService!!!!!
    sentCfgs++;
  }
  newMsg->addKeyValuePair("SentConfigs", sentCfgs);
  commsService->sendMessage(*newMsg, LFAST::CommsService::ACTIVE_CONNECTION);
  sentMessages++;
  cli->printfDebugMessage("Sent %d configs over %d messages.", sentCfgs, sentMessages);

  delete newMsg;
}