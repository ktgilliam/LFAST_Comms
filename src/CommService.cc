
#include "CommService.h"

#include <array>
#include <sstream>
#include <iterator>
#include <cstring>
#include <string>
#include <cstdlib>
#include <StreamUtils.h>
#include <algorithm>
// #include <string_view>

#include <debug.h>
#include <device.h>

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
    TEST_SERIAL.printf("Unregistered Message: [%s].\r\n", info.c_str());
}

void LFAST::CommsService::errorMessageHandler(CommsMessage &msg)
{
    std::stringstream ss;
    ss << "Invalid Message.";
    //
    ss << std::endl;
    TEST_SERIAL.println(ss.str().c_str());
}

bool LFAST::CommsService::checkForNewClients()
{
    bool newClientFlag = false;
    // TODO
    return (newClientFlag);
}

// void LFAST::CommsMessage::printMessageInfo() {}
void LFAST::CommsService::checkForNewClientData()
{
    // check for incoming data from all clients
    for (auto &connection : this->connections)
    {
        if (connection.client->available())
        {
            getNewMessages(connection);
        }
    }
}

bool LFAST::CommsService::getNewMessages(ClientConnection &connection)
{
    // static unsigned int msgCount = 0;
    // listen for incoming clients
    Client *client = connection.client;
    if (client)
    {
        // TEST_SERIAL.println("\r\n");
        // TEST_SERIAL.printf("\r\n%u:", msgCount++);
        // CommsMessage newMsg;
        auto newMsg = new CommsMessage();
        unsigned int bytesRead = 0;
        bool readingObject = false, objectDone = false;
        int openObjectsCnt = 0;

        while (client->connected())
        {
            // TEST_SERIAL.println("Checking connected client messages");
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
                // TEST_SERIAL.print(c);
                if (objectDone)
                {
                    newMsg->jsonInputBuffer[bytesRead + 1] = '\0';
                    // TEST_SERIAL.printf(" [%u bytes]\r\n", bytesRead);

                    connection.rxMessageQueue.push_back(newMsg);
                    // TEST_SERIAL.println("Received Message:");
                    // newMsg->printMessageInfo();
                    break;
                }
            }
        }
    }
    return true;
}

void LFAST::CommsMessage::printMessageInfo()
{
    CURSOR_TO_DEBUG_ROW(-3);
    TEST_SERIAL.printf("MESSAGE ID: %u\033[0K\r\n", (unsigned int)this->getBuffPtr());
    // TEST_SERIAL.printf("\tMESSAGE Input Buffer: %s (%u bytes)\r\n", this->jsonInputBuffer, std::strlen(this->jsonInputBuffer));
    TEST_SERIAL.print("MESSAGE Input Buffer: \033[0K");

    bool nullTermFound = false;
    unsigned int ii = 0;
    while (!nullTermFound && ii < JSON_PROGMEM_SIZE)
    {
        char c2 = this->jsonInputBuffer[ii++];
        if (c2 != '\0')
        {
            TEST_SERIAL.printf("%c", c2);
        }
        else
        {
            nullTermFound = true;
            TEST_SERIAL.printf("%s[%u]\r\n", "\\0", ii);
        }
    }

    // serializeJson(jsonDoc, TEST_SERIAL);
    // TEST_SERIAL.printf("\tBytes: %u\r\n", measureJson(jsonDoc));
    TEST_SERIAL.println("");
}

void LFAST::CommsService::processClientData()
{
    for (auto &conn : this->connections)
    {
        this->activeConnection = &conn;
        auto itr = conn.rxMessageQueue.begin();
        while (itr != conn.rxMessageQueue.end())
        {
            processMessage(*itr);
            delete *itr;
            itr = conn.rxMessageQueue.erase(itr);
        }
    }
    this->activeConnection = nullptr;
}
void LFAST::CommsService::processMessage(CommsMessage *msg)
{
    if (msg->hasBeenProcessed())
    {
        TEST_SERIAL.println("Something went wrong processing messages.");
        return;
    }

    msg->printMessageInfo();
    StaticJsonDocument<JSON_PROGMEM_SIZE> &doc = msg->deserialize();
    JsonObject msgRoot = doc.as<JsonObject>();
    JsonObject msgObject = msgRoot["MountMessage"];
    // Test if parsing succeeds.

    for (JsonPair kvp : msgObject)
    {
        // auto keyStr = kvp.key().c_str();
        // TEST_SERIAL.printf("Processing Key: %s\r\n", keyStr);
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
    // TEST_SERIAL.println("Sending message:");
    // msg.printMessageInfo();
    if (sendOpt == ACTIVE_CONNECTION)
    {
        WriteBufferingStream bufferedClient(*(activeConnection->client), std::strlen(msg.getBuffPtr()));
        serializeJson(msg.getJsonDoc(), bufferedClient);
        bufferedClient.flush();
        activeConnection->client->write('\0');
    }
    else
    {
        // TODO
        TEST_SERIAL.println("Not yet implemented (something went wrong).");
    }
}

StaticJsonDocument<JSON_PROGMEM_SIZE> &LFAST::CommsMessage::deserialize()
{
    DeserializationError error = deserializeJson(this->JsonDoc, this->jsonInputBuffer);
    if (error)
    {
        TEST_SERIAL.print(F("deserializeJson() failed: "));
        TEST_SERIAL.println(error.f_str());
    }
    return this->JsonDoc;
}
void LFAST::CommsService::stopDisconnectedClients()
{
    auto itr = connections.begin();
    while (itr != connections.end())
    {
        if (!(*itr).client->connected())
        {
            // TEST_SERIAL.println("Disconnected client.");
            (*itr).client->stop();
            itr = connections.erase(itr);
        }
        else
        {
            itr++;
        }
    }
}
