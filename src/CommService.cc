
#include "../include/CommService.h"

#include <Arduino.h>
#include <array>
#include <sstream>
#include <iterator>
#include <cstring>
#include <string>
#include <cstdlib>
#include <StreamUtils.h>
#include <algorithm>
// #include <string_view>

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

void LFAST::CommsService::defaultMessageHandler(std::string info)
{
    Serial2.printf("Unregistered Message: [%s].\r\n", info.c_str());
}

void LFAST::CommsService::errorMessageHandler(CommsMessage &msg)
{
    std::stringstream ss;
    if (cli != nullptr)
    {
        char debugMsgBuff[100+JSON_PROGMEM_SIZE]{0};
        char msgBuff[JSON_PROGMEM_SIZE]{0};
        msg.getMessageStr(msgBuff);
        sprintf(debugMsgBuff, "Invalid Message: %s", msgBuff);
        cli->printDebugMessage(msgBuff);
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

void LFAST::CommsMessage::printMessageInfo(TerminalInterface *debugCli)
{
    if (debugCli != nullptr)
    {
        char msgBuff[100]{0};

        sprintf(msgBuff, "MESSAGE ID: %u\033[0K\r\n", (unsigned int)this->getBuffPtr());
        debugCli->printDebugMessage(msgBuff);
        std::memset(msgBuff, 0, sizeof(msgBuff));

        sprintf(msgBuff, "MESSAGE Input Buffer: \033[0K");
        debugCli->printDebugMessage(msgBuff);
        std::memset(msgBuff, 0, sizeof(msgBuff));

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

void LFAST::CommsService::processClientData(const std::string &destFilter = "")
{
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
    this->activeConnection = nullptr;
}

void LFAST::CommsService::processMessage(CommsMessage *msg, const std::string &destFilter)
{
    if (msg->hasBeenProcessed())
    {
        if (cli != nullptr)
        {
            cli->printDebugMessage("Something went wrong processing messages.");
        }
        return;
    }

    StaticJsonDocument<JSON_PROGMEM_SIZE> &doc = msg->deserialize();
    JsonObject msgRoot = doc.as<JsonObject>();
    if (!destFilter.empty())
        msgRoot = msgRoot[destFilter];
    // Test if parsing succeeds.
    if (cli != nullptr)
    {
        StaticJsonDocument<JSON_PROGMEM_SIZE> docCopy = msgRoot;
        char printBuff[JSON_PROGMEM_SIZE]{0};
        serializeJson(docCopy, printBuff, JSON_PROGMEM_SIZE);
        cli->updatePersistentField(DeviceName, PROCESSED_MESSAGE_ROW, printBuff);
    }
    for (JsonPair kvp : msgRoot)
    {
        this->callMessageHandler(kvp);
    }
    msg->setProcessedFlag();
}


bool LFAST::CommsService::callMessageHandler(JsonPair kvp)
{
    bool handlerFound = true;
    auto keyStr = std::string(kvp.key().c_str());
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
            auto val = kvp.value().as<std::string>();
            this->callMessageHandler<std::string>(keyStr, val);
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
    if (sendOpt == ACTIVE_CONNECTION)
    {
        if (cli != nullptr)
        {
            char msgBuff[JSON_PROGMEM_SIZE]{0};
            msg.getMessageStr(msgBuff);
            cli->updatePersistentField(DeviceName, MESSAGE_SENT_ROW, msgBuff);
        }
        WriteBufferingStream bufferedClient(*(activeConnection->client), std::strlen(msg.getBuffPtr()));
        serializeJson(msg.getJsonDoc(), bufferedClient);
        bufferedClient.flush();
        activeConnection->client->write('\0');
    }
    else
    {
        if (cli != nullptr)
        {
            cli->printDebugMessage("Not yet implemented (something went wrong).");
        }
    }
}

StaticJsonDocument<JSON_PROGMEM_SIZE> &LFAST::CommsMessage::deserialize(TerminalInterface *debugCli)
{
    DeserializationError error = deserializeJson(this->JsonDoc, this->jsonInputBuffer);
    if (error)
    {
        if (debugCli != nullptr)
        {
            char msgBuff[100]{0};
            sprintf(msgBuff, "deserializeJson() failed: %s", error.c_str());
            debugCli->printDebugMessage(msgBuff);
        }
    }
    return this->JsonDoc;
}

void LFAST::CommsMessage::getMessageStr(char *buff)
{
    StaticJsonDocument<JSON_PROGMEM_SIZE> docCopy = this->JsonDoc;
    serializeJson(docCopy, buff, JSON_PROGMEM_SIZE);
}

void LFAST::CommsService::stopDisconnectedClients()
{
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

void LFAST::CommsService::setupPersistentFields()
{
    if (cli == nullptr)
        return;
    cli->addPersistentField(this->DeviceName,"[STATUS]", COMMS_SERVICE_STATUS_ROW);

    cli->addPersistentField(this->DeviceName,"[RAW RX]", RAW_MESSAGE_RECEIVED_ROW);

    cli->addPersistentField(this->DeviceName,"[PROCESSED RX]", PROCESSED_MESSAGE_ROW);

    cli->addPersistentField(this->DeviceName,"[TX]", MESSAGE_SENT_ROW);
}

// void LFAST::CommsService::updateStatusFields()
// {
//     cli->updatePersistentField(COMMS_SERVICE_STATUS_ROW, "Status not set up yet");
//     cli->updatePersistentField(MESSAGE_RECEIVED_ROW, "Status not set up yet");
//     cli->updatePersistentField(MESSAGE_SENT_ROW, "Status not set up yet");
// }