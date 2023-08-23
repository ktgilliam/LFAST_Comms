/*******************************************************************************
Copyright 2022
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
///
/// @author Kevin Gilliam
/// @date February 16th, 2023
/// @file CommService.h
///
/// The LFAST Comms library (of which TcpCommsService is a component)
/// works by associating JSON key-value pairs to function pointers.
/// When a key is received, the library checks to see if a function
/// pointer has been registered for it. If it has, it calls that function
/// and passes the value from the key-value pair as an argument.
/// This template defines and registers two such callbacks in this file.
///

#pragma once
#include "LFAST_Device.h"
#include <Arduino.h>
#include <StreamUtils.h>
#include "Client.h"
#include <ArduinoJson.h>

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <cstring>
#include "teensy41_device.h"

#define MAX_ARGS 4
#define RX_BUFF_SIZE 1024

#define MAX_KV_PAIRS 20
#define JSON_PROGMEM_SIZE JSON_OBJECT_SIZE(MAX_KV_PAIRS)

#define MAX_CTRL_MESSAGES 0x40U // can be increased if needed

enum COMMS_SERVICE_INFO_ROWS
{
    COMMS_SERVICE_STATUS_ROW,
    RAW_MESSAGE_RECEIVED_ROW,
    PROCESSED_MESSAGE_ROW,
    MESSAGE_SENT_ROW
    //     // PROMPT_ROW,
    //     // PROMPT_FEEDBACK,
    // #if PRINT_SERVICE_COUNTER
    //     SERVICE_COUNTER_ROW,
    // #endif
    //     DEBUG_BORDER_1,
    //     DEBUG_MESSAGE_ROW
};
namespace LFAST
{
    enum MESSAGE_TYPE
    {
        ARRAY_MESSAGE,
        OBJECT_MESSAGE
    };
    ///////////////// TYPES /////////////////
    class CommsMessage
    {
    public:
        CommsMessage()
            : JsonDoc(DynamicJsonDocument(JSON_PROGMEM_SIZE))
        {
            std::memset(this->jsonInputBuffer, 0, sizeof(this->jsonInputBuffer));
            processed = false;
        }
        virtual ~CommsMessage() {}
        virtual void placeholder() {}

        void getMessageStr(char *);
        DynamicJsonDocument &getJsonDoc()
        {
            return this->JsonDoc;
        }
#if defined(TERMINAL_ENABLED)
        DynamicJsonDocument &deserialize(TerminalInterface *debugCli = nullptr);
#else
        DynamicJsonDocument &deserialize();
#endif
        template <typename T>
        inline T getValue(const char *key);

        template <typename T>
        inline void addKeyValuePair(const char *key, T val);
        inline bool startNewArrayObject();
        inline bool startNewArray(const char *key);
        template <typename T>
        inline void addKeyValuePairToArray(const char *key, T val);
        // inline void addKeyValuePair(const char *  key, T val);
        inline void addDestinationKey(const char *key){};
        // bool isMessageFull();
        const char *getBuffPtr()
        {
            return jsonInputBuffer;
        };
        char jsonInputBuffer[JSON_PROGMEM_SIZE];
        void setProcessedFlag()
        {
            processed = true;
        }
        bool hasBeenProcessed()
        {
            return processed;
        }

#if defined(TERMINAL_ENABLED)
        void printMessageInfo(TerminalInterface *debugCli = nullptr);
#endif

    protected:
        DynamicJsonDocument JsonDoc;
        bool processed;
        JsonArray array;
        JsonObject nested;
        std::string destKey;
        std::string arrayKey;
        size_t arrayMemUsagePrev;
    };

    template <class T>
    struct MessageHandler
    {
        void (*MsgHandlerFn)(T);

        MessageHandler()
        {
            this->MsgHandlerFn = nullptr;
        }

        MessageHandler(void (*ptr)(T))
        {
            this->MsgHandlerFn = ptr;
        }
        virtual ~MessageHandler(){};

        bool call(T val)
        {
            if (this->MsgHandlerFn)
            {
                MsgHandlerFn(val);
                return true;
            }
            return false;
        }
    };

    struct ClientConnection
    {
        ClientConnection(Client *_client) : client(_client), noReplyFlag(false) {}
        Client *client;
        bool noReplyFlag;
        std::vector<CommsMessage *> rxMessageQueue;
        std::vector<CommsMessage *> txMessageQueue;
    };

    class CommsService : public LFAST_Device
    {

    protected:
        static void defaultMessageHandler(const char *);
        void errorMessageHandler(CommsMessage &msg);
        static std::vector<ClientConnection> connections;
        ClientConnection *activeConnection;
        bool commsServiceStatus;

#if defined(TERMINAL_ENABLED)
        virtual void setupPersistentFields() override;
#endif
    private:
        enum HandlerType
        {
            INT_HANDLER,
            UINT_HANDLER,
            FLOAT_HANDLER,
            DOUBLE_HANDLER,
            BOOL_HANDLER,
            STRING_HANDLER
        };
        std::unordered_map<std::string, HandlerType> handlerTypes;
        std::unordered_map<std::string, MessageHandler<int>> intHandlers;
        std::unordered_map<std::string, MessageHandler<unsigned int>> uIntHandlers;
        std::unordered_map<std::string, MessageHandler<float>> floatHandlers;
        std::unordered_map<std::string, MessageHandler<double>> doubleHandlers;
        std::unordered_map<std::string, MessageHandler<bool>> boolHandlers;
        // std::unordered_map<std::string, MessageHandler<std::string>> stringHandlers;
        std::unordered_map<std::string, MessageHandler<const char *>> stringHandlers;
        template <class T>
        bool callMessageHandler(const char *key, T val);

    public:
        CommsService();
        virtual ~CommsService() {}

        void setupClientMessageBuffers(Client *client);
        bool getNewMessages(ClientConnection &);
        enum
        {
            ACTIVE_CONNECTION = 1,
            ALL_CONNECTED = 2,
        };
        virtual void sendMessage(CommsMessage &, uint8_t);
        template <class T>
        inline bool registerMessageHandler(const char *key, MessageHandler<T> fn);
        inline bool callMessageHandler(JsonPair kvp);

        virtual bool Status()
        {
            return commsServiceStatus;
        };

        bool checkForNewClientData();
        virtual bool checkForNewClients();
        virtual void stopDisconnectedClients();
        virtual void processClientData(const char *);
        virtual void processMessage(CommsMessage *, const char *);
        void setNoReplyFlag(bool f)
        {
            activeConnection->noReplyFlag = f;
        }
    };

    // NOTE: Teensy build environment doesn't handle build flags properly, so can't use typeid().
    // template <class T>
    // bool LFAST::CommsService::registerMessageHandler(const char *  key, MessageHandler<T> fn)
    // {
    //     if (typeid(T) == typeid(int))
    //         this->intHandlers[key] = fn;
    //     else if (typeid(T) == typeid(unsigned int))
    //         this->uIntHandlers[key] = fn;
    //     else if (typeid(T) == typeid(double))
    //         this->doubleHandlers[key] = fn;
    //     else if (typeid(T) == typeid(bool))
    //         this->boolHandlers[key] = fn;
    //     else if (typeid(T) == typeid(std::string))
    //         this->stringHandlers[key] = fn;
    //     else
    //         return false;
    //     return true;
    // }

    template <class T>
    bool LFAST::CommsService::registerMessageHandler(const char *key, MessageHandler<T> fn)
    {
        // TODO: Add exception handling
        return false;
    }

    template <>
    inline bool LFAST::CommsService::registerMessageHandler(const char *key, MessageHandler<int> fn)
    {
        this->intHandlers[key] = fn;
        this->handlerTypes[key] = INT_HANDLER;
        return true;
    }
    template <>
    inline bool LFAST::CommsService::registerMessageHandler(const char *key, MessageHandler<unsigned int> fn)
    {
        this->uIntHandlers[key] = fn;
        this->handlerTypes[key] = UINT_HANDLER;
        return true;
    }
    template <>
    inline bool LFAST::CommsService::registerMessageHandler(const char *key, MessageHandler<float> fn)
    {
        this->floatHandlers[key] = fn;
        this->handlerTypes[key] = FLOAT_HANDLER;
        return true;
    }
    template <>
    inline bool LFAST::CommsService::registerMessageHandler(const char *key, MessageHandler<double> fn)
    {
        this->doubleHandlers[key] = fn;
        this->handlerTypes[key] = DOUBLE_HANDLER;
        return true;
    }
    template <>
    inline bool LFAST::CommsService::registerMessageHandler(const char *key, MessageHandler<bool> fn)
    {
        this->boolHandlers[key] = fn;
        this->handlerTypes[key] = BOOL_HANDLER;
        return true;
    }

    template <>
    inline bool LFAST::CommsService::registerMessageHandler(const char *key, MessageHandler<const char *> fn)
    {
        this->stringHandlers[key] = fn;
        this->handlerTypes[key] = STRING_HANDLER;
        return true;
    }
    template <>
    inline bool LFAST::CommsService::callMessageHandler(const char *key, int val)
    {
        auto mh = this->intHandlers[key];
        mh.call(val);
        return true;
    }

    template <>
    inline bool LFAST::CommsService::callMessageHandler(const char *key, unsigned int val)
    {
        auto mh = this->uIntHandlers[key];
        mh.call(val);
        return true;
    }

    template <>
    inline bool LFAST::CommsService::callMessageHandler(const char *key, float val)
    {
        auto mh = this->floatHandlers[key];
        mh.call(val);
        return true;
    }

    template <>
    inline bool LFAST::CommsService::callMessageHandler(const char *key, double val)
    {
        auto mh = this->doubleHandlers[key];
        mh.call(val);
        return true;
    }

    template <>
    inline bool LFAST::CommsService::callMessageHandler(const char *key, bool val)
    {
        auto mh = this->boolHandlers[key];
        mh.call(val);
        return true;
    }

    template <>
    inline bool LFAST::CommsService::callMessageHandler(const char *key, const char *val)
    {
        auto mh = this->stringHandlers[key];
        mh.call(val);
        return true;
    }

    template <>
    inline double CommsMessage::getValue(const char *key)
    {
        return (JsonDoc[key].as<double>());
    }

    template <>
    inline int CommsMessage::getValue(const char *key)
    {
        return (JsonDoc[key].as<int>());
    }

    template <>
    inline unsigned int CommsMessage::getValue(const char *key)
    {
        return (JsonDoc[key].as<unsigned int>());
    }

    template <>
    inline bool CommsMessage::getValue(const char *key)
    {
        return (JsonDoc[key].as<bool>());
    }

    template <>
    inline const char *CommsMessage::getValue(const char *key)
    {
        return (JsonDoc[key].as<const char *>());
    }

    // inline void CommsMessage::addDestinationKey(const char *  key)
    // {

    //     // newDoc.createNestedObject(key);
    //     // newDoc[key] = JsonDoc.as<JsonObject>();
    //     destKey = key;

    //     if (!JsonDoc.isNull())
    //     {
    //         DynamicJsonDocument<JSON_PROGMEM_SIZE> newDoc;

    //         for (JsonPairConst kvp : this->JsonDoc.as<JsonObjectConst>())
    //         {
    //             newDoc[key][kvp.key()] = kvp.value();
    //         }
    //         this->JsonDoc = newDoc;
    //     }
    //     else
    //     {
    //         JsonDoc.createNestedObject(destKey);
    //     }

    //     // = this->getJsonDoc();
    //     // JsonObject newObj = JsonDoc.
    // }

    // template <typename T>
    // inline void CommsMessage::addKeyValuePair(const char *  key, T val)
    // {
    //     if (this->destKey.length() > 0)
    //         JsonDoc[(this->destKey)][key] = val;
    //     else
    //         JsonDoc[key] = val;
    // };

    template <typename T>
    inline void CommsMessage::addKeyValuePair(const char *key, T val)
    {
        if (this->destKey.length() > 0)
            JsonDoc[(this->destKey)][(key)] = val;
        else
            JsonDoc[(key)] = val;
    }

    template <typename T>
    inline void CommsMessage::addKeyValuePairToArray(const char *key, T val)
    {
        // if (msgIsArray && !nested.isNull())
        if (JsonDoc.is<JsonArray>() && !nested.isNull())
        {
            nested[(key)] = val;
        }
    }

    inline bool CommsMessage::startNewArray(const char *key)
    {
        // msgIsArray = true;
        arrayKey = key;
        // array = JsonDoc.to<JsonArray>();
        array = JsonDoc.createNestedArray(arrayKey);
        arrayMemUsagePrev = 0;
        return true;
    }

    inline bool CommsMessage::startNewArrayObject()
    {
        bool success = false;
        // if (!JsonDoc.is<JsonArray>())
        if (array.isNull())
        {
            arrayMemUsagePrev = 0;
            array = JsonDoc.to<JsonArray>();
        }

        // size_t arrLen = 0;
        size_t arrayMemUsageCurr;
        size_t memUsageDiff;
        if (nested.isNull())
        {
            nested = array.createNestedObject();
            success = true;
        }
        else
        {
            arrayMemUsageCurr = array.memoryUsage();
            memUsageDiff = arrayMemUsageCurr - arrayMemUsagePrev;
            arrayMemUsagePrev = arrayMemUsageCurr;
            bool test0 = arrayMemUsageCurr >= JSON_PROGMEM_SIZE;
            bool test1 = (arrayMemUsageCurr + memUsageDiff) > JSON_PROGMEM_SIZE;
            if (test0 || test1)
            {
                success = false;
            }
            else
            {
                nested = array.createNestedObject();
                success = true;
            }
        }
        return success;
    }

};