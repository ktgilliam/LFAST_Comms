
#include "../include/CommService.h"

#include <Arduino.h>
#include <array>
#include <iterator>
#include <cstring>
// #include <string>
#include <cstdlib>
#include <StreamUtils.h>
#include <algorithm>
#include "teensy41_device.h"

std::vector<LFAST::ClientConnection> LFAST::CommsService::connections{};
///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
LFAST::CommsService::CommsService()
{
    activeConnection = nullptr;
}

void LFAST::CommsService::setupClientMessageBuffers(Client *client)
{
    // ClientConnection is created on the stack
    ClientConnection newConnection(client);
    this->connections.push_back(newConnection);
}

void LFAST::CommsService::defaultMessageHandler(const char *info)
{
    if (cli != nullptr)
    {
        cli->printfDebugMessage("Unregistered Message: [%s].\r\n", info);
    }
}

void LFAST::CommsService::errorMessageHandler(CommsMessage &msg)
{
    if (cli != nullptr)
    {
        char msgBuff[JSON_PROGMEM_SIZE]{0};
        msg.getMessageStr(msgBuff);
        cli->printfDebugMessage("Invalid Message: %s", msgBuff);
    }
}

bool LFAST::CommsService::checkForNewClients()
{
    bool newClientFlag = false;
    // TODO
    return (newClientFlag);
}

bool LFAST::CommsService::checkForNewClientData()
{
    // if (cli != nullptr)
    //     cli->updatePersistentField(DeviceName, COMMS_SERVICE_STATUS_ROW, "checkForNewClientData()");
    bool newMsgFlag = false;
    // check for incoming data from all clients
    for (auto &connection : this->connections)
    {
        if (connection.client->available())
        {
            newMsgFlag = getNewMessages(connection);
        }
    }
    return newMsgFlag;
}

bool LFAST::CommsService::getNewMessages(ClientConnection &connection)
{

    // if (cli != nullptr)
    //     cli->updatePersistentField(DeviceName, COMMS_SERVICE_STATUS_ROW, "getNewMessages()");
    // listen for incoming clients
    Client *client = connection.client;
    if (client)
    {
        auto newMsg = new CommsMessage();
        unsigned int bytesRead = 0;
        bool readingObject = false, objectDone = false;
        int openObjectsCnt = 0;

        while (client->connected())
        {
            if (client->available())
            {
                char c = client->read();
                newMsg->jsonInputBuffer[bytesRead++] = c;
                if (c == '{')
                {
                    if (!readingObject)
                        readingObject = true;
                    openObjectsCnt++;
                }
                if ((c == '}') && (readingObject))
                {
                    readingObject = true;
                    openObjectsCnt--;
                    if (openObjectsCnt == 0)
                        objectDone = true;
                }
                if (objectDone)
                {
                    newMsg->jsonInputBuffer[bytesRead + 1] = '\0';
                    if (cli != nullptr)
                    {
                        cli->updatePersistentField(DeviceName, RAW_MESSAGE_RECEIVED_ROW, newMsg->jsonInputBuffer);
                    }
                    connection.rxMessageQueue.push_back(newMsg);
                    break;
                }
            }
        }
    }
    return true;
}

#if defined(TERMINAL_ENABLED)
void LFAST::CommsMessage::printMessageInfo(TerminalInterface *debugCli)
{
    if (debugCli != nullptr)
    {
        debugCli->printfDebugMessage("MESSAGE ID: %u\033[0K\r\n", (unsigned int)this->getBuffPtr());
        debugCli->printDebugMessage("MESSAGE Input Buffer: \033[0K");

        // bool nullTermFound = false;
        // unsigned int ii = 0;
        // while (!nullTermFound && ii < JSON_PROGMEM_SIZE)
        // {
        //     char c2 = this->jsonInputBuffer[ii++];
        //     if (c2 != '\0')
        //     {
        //         Serial2.printf("%c", c2);
        //     }
        //     else
        //     {
        //         nullTermFound = true;
        //         Serial2.printf("%s[%u]\r\n", "\\0", ii);
        //     }
        // }
        // Serial2.println("");
    }
}
#endif

void LFAST::CommsService::processClientData(const char *destFilter = "")
{
    // if (cli != nullptr)
    //     cli->updatePersistentField(DeviceName, COMMS_SERVICE_STATUS_ROW, "processClientData()");
    for (auto &conn : this->connections)
    {
        this->activeConnection = &conn;
        auto itr = conn.rxMessageQueue.begin();
        while (itr != conn.rxMessageQueue.end())
        {
            processMessage(*itr, destFilter);
            delete *itr;
            itr = conn.rxMessageQueue.erase(itr);
        }
    }
    // this->activeConnection = nullptr;
}

void LFAST::CommsService::processMessage(CommsMessage *msg, const char *destFilter)
{
    // if (cli != nullptr)
    //     cli->updatePersistentField(DeviceName, COMMS_SERVICE_STATUS_ROW, "processMessage()");
    if (msg->hasBeenProcessed())
    {
#if defined(TERMINAL_ENABLED)
        if (cli != nullptr)
        {
            cli->printDebugMessage("Something went wrong processing messages.");
        }
#endif
        return;
    }
#if defined(TERMINAL_ENABLED)
    if (cli != nullptr)
    {
        cli->updatePersistentField(DeviceName, PROCESSED_MESSAGE_ROW, msg->jsonInputBuffer);
    }
#endif
    DynamicJsonDocument &doc = msg->deserialize();
    JsonObject msgRoot = doc.as<JsonObject>();

    // Test if parsing succeeds.
    if (strlen(destFilter) > 0)
        msgRoot = msgRoot[destFilter];
    for (JsonPair kvp : msgRoot)
    {
        this->callMessageHandler(kvp);
    }

    // memset(msg->jsonInputBuffer, 0, JSON_PROGMEM_SIZE);
    msg->setProcessedFlag();
}

bool LFAST::CommsService::callMessageHandler(JsonPair kvp)
{
    // if (cli != nullptr)
    //     cli->updatePersistentField(DeviceName, COMMS_SERVICE_STATUS_ROW, "callMessageHandler()");
    bool handlerFound = true;
    auto keyStr = kvp.key().c_str();
    if (this->handlerTypes.find(keyStr) == this->handlerTypes.end())
    {
        handlerFound = false;
        defaultMessageHandler(keyStr);
    }
    else
    {
        auto handlerType = this->handlerTypes[keyStr];

        switch (handlerType)
        {
        case INT_HANDLER:
        {
            auto val = kvp.value().as<int>();
            this->callMessageHandler<int>(keyStr, val);
        }
        break;
        case UINT_HANDLER:
        {
            auto val = kvp.value().as<unsigned int>();
            this->callMessageHandler<unsigned int>(keyStr, val);
        }
        break;
        case DOUBLE_HANDLER:
        {
            auto val = kvp.value().as<double>();
            this->callMessageHandler<double>(keyStr, val);
        }
        break;
        case BOOL_HANDLER:
        {
            auto val = kvp.value().as<bool>();
            this->callMessageHandler<bool>(keyStr, val);
        }
        break;
        case STRING_HANDLER:
        {
            auto val = kvp.value().as<const char *>();
            this->callMessageHandler<const char *>(keyStr, val);
        }
        break;
        default:
            handlerFound = false;
        }
    }

    return handlerFound;
}

void LFAST::CommsService::sendMessage(CommsMessage &msg, uint8_t sendOpt)
{
    static int callCount = 0;
#if defined(TERMINAL_ENABLED)
    if (cli != nullptr)
        cli->updatePersistentField(DeviceName, COMMS_SERVICE_STATUS_ROW, callCount++);
#endif
    if (sendOpt == ACTIVE_CONNECTION)
    {
        if (activeConnection == nullptr)
        {
#if defined(TERMINAL_ENABLED)            
            if (cli != nullptr)
            {
                cli->printDebugMessage("Error: Active connection is null", LFAST::ERROR_MESSAGE);
            }
#endif
            return;
        }
#if defined(TERMINAL_ENABLED)
        if (cli != nullptr)
        {
            char msgBuff[JSON_PROGMEM_SIZE]{0};
            msg.getMessageStr(msgBuff);
            cli->updatePersistentField(DeviceName, MESSAGE_SENT_ROW, msgBuff);
        }
#endif
        if (activeConnection->client)
        {
#define USE_BUFFERED_CLIENT 1
#if USE_BUFFERED_CLIENT == 1
            auto sz = measureJson(msg.getJsonDoc());
            auto chunkSize = sz < 64 ? sz : 64;
            WriteBufferingStream bufferedClient{*(activeConnection->client), chunkSize};
            serializeJson(msg.getJsonDoc(), bufferedClient);
            bufferedClient.flush();
            activeConnection->client->write('\0');
#else
            serializeJson(msg.getJsonDoc(), *(activeConnection->client));
            activeConnection->client->write('\0');
            // cli->printDebugMessage("Done sending (unbuff'd)");
#endif
        }
    }
    else
    {
#if defined(TERMINAL_ENABLED)
        if (cli != nullptr)
        {
            cli->printDebugMessage("Not yet implemented (something went wrong).");
        }
#endif
    }
}

#if defined(TERMINAL_ENABLED)
DynamicJsonDocument &LFAST::CommsMessage::deserialize(TerminalInterface *debugCli)
#else
DynamicJsonDocument &LFAST::CommsMessage::deserialize()
#endif
{
    if (this->jsonInputBuffer != nullptr)
    {
        DeserializationError error = deserializeJson(this->JsonDoc, this->jsonInputBuffer);
#if defined(TERMINAL_ENABLED)
        if (error)
        {
            if (debugCli != nullptr)
            {
                debugCli->printfDebugMessage("deserializeJson() failed: %s", error.c_str());
            }
        }
#endif
    }
    else
    {
#if defined(TERMINAL_ENABLED)
        debugCli->printfDebugMessage("jsonInputBuffer nullptr error");
#endif
        while (1) {;}
    }
    return this->JsonDoc;
}

void LFAST::CommsMessage::getMessageStr(char *buff)
{
    // TEST_SERIAL.println("getMessageStr");
    DynamicJsonDocument docCopy(JSON_PROGMEM_SIZE);
    docCopy = this->JsonDoc;
    serializeJson(docCopy, buff, JSON_PROGMEM_SIZE);
}

void LFAST::CommsService::stopDisconnectedClients()
{
    // if (cli != nullptr)
    //     cli->updatePersistentField(DeviceName, COMMS_SERVICE_STATUS_ROW, "stopDisconnectedClients()");
    auto itr = connections.begin();
    while (itr != connections.end())
    {
        if (!(*itr).client->connected())
        {
            (*itr).client->stop();
            itr = connections.erase(itr);
        }
        else
        {
            itr++;
        }
    }
}
#if defined(TERMINAL_ENABLED)
void LFAST::CommsService::setupPersistentFields()
{
    if (cli == nullptr)
        return;
    cli->addPersistentField(this->DeviceName, "[STATUS]", COMMS_SERVICE_STATUS_ROW);

    cli->addPersistentField(this->DeviceName, "[RAW RX]", RAW_MESSAGE_RECEIVED_ROW);

    cli->addPersistentField(this->DeviceName, "[PROCESSED RX]", PROCESSED_MESSAGE_ROW);

    cli->addPersistentField(this->DeviceName, "[TX]", MESSAGE_SENT_ROW);
}
#endif
// void LFAST::CommsService::updateStatusFields()
// {
//     cli->updatePersistentField(COMMS_SERVICE_STATUS_ROW, "Status not set up yet");
//     cli->updatePersistentField(MESSAGE_RECEIVED_ROW, "Status not set up yet");
//     cli->updatePersistentField(MESSAGE_SENT_ROW, "Status not set up yet");
// }