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
///////////////// TYPES /////////////////
class CommsMessage
{
    public:
        // CommsMessage(){}
        CommsMessage()
        {
            std::memset(this->jsonInputBuffer, 0, sizeof(this->jsonInputBuffer));
            processed = false;
        }
        virtual ~CommsMessage() {} 
        virtual void placeholder() {}
        void printMessageInfo(TerminalInterface * debugCli=nullptr);
        void getMessageStr(char *);
        StaticJsonDocument<JSON_PROGMEM_SIZE> &getJsonDoc()
        {
            return this->JsonDoc;
        }
        StaticJsonDocument<JSON_PROGMEM_SIZE> &deserialize(TerminalInterface * debugCli=nullptr);

        template <typename T>
        inline T getValue(const char * key);
    
        template <typename T>
        inline void addKeyValuePair(const char * key, T val);
        // inline void addKeyValuePair(const char *  key, T val);
        inline void addDestinationKey(const char *  key);

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
    protected:
        StaticJsonDocument<JSON_PROGMEM_SIZE> JsonDoc;
        bool processed;
        std::string destKey;
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
    virtual ~MessageHandler() {};

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
    std::vector<CommsMessage*> rxMessageQueue;
    std::vector<CommsMessage*> txMessageQueue;
};

class CommsService : public LFAST_Device
{

    protected:
        static void defaultMessageHandler(const char *);
        void errorMessageHandler(CommsMessage &msg);
        static std::vector<ClientConnection> connections;
        ClientConnection *activeConnection;
        bool commsServiceStatus;
        virtual void setupPersistentFields() override;
        
    private:
        enum HandlerType
        {
            INT_HANDLER,
            UINT_HANDLER,
            DOUBLE_HANDLER,
            BOOL_HANDLER,
            STRING_HANDLER
        };
        std::unordered_map<std::string, HandlerType> handlerTypes;
        std::unordered_map<std::string, MessageHandler<int>> intHandlers;
        std::unordered_map<std::string, MessageHandler<unsigned int>> uIntHandlers;
        std::unordered_map<std::string, MessageHandler<double>> doubleHandlers;
        std::unordered_map<std::string, MessageHandler<bool>> boolHandlers;
        std::unordered_map<std::string, MessageHandler<std::string>> stringHandlers;
        template <class T>
        bool callMessageHandler(const char *  key, T val);

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
        inline bool registerMessageHandler(const char *  key, MessageHandler<T> fn);
        inline bool callMessageHandler(JsonPair kvp);

        virtual bool Status()
        {
            return commsServiceStatus;
        };

        bool checkForNewClientData();
        virtual bool checkForNewClients();
        virtual void stopDisconnectedClients();
        virtual void processClientData(const char * );
        virtual void processMessage(CommsMessage *, const char * );
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
bool LFAST::CommsService::registerMessageHandler(const char *  key, MessageHandler<T> fn)
{
    // TODO: Add exception handling
    return false;
}

template <>
inline bool LFAST::CommsService::registerMessageHandler(const char *  key, MessageHandler<int> fn)
{
    this->intHandlers[key] = fn;
    this->handlerTypes[key] = INT_HANDLER;
    return true;
}
template <>
inline bool LFAST::CommsService::registerMessageHandler(const char *  key, MessageHandler<unsigned int> fn)
{
    this->uIntHandlers[key] = fn;
    this->handlerTypes[key] = UINT_HANDLER;
    return true;
}
template <>
inline bool LFAST::CommsService::registerMessageHandler(const char *  key, MessageHandler<double> fn)
{
    this->doubleHandlers[key] = fn;
    this->handlerTypes[key] = DOUBLE_HANDLER;
    return true;
}
template <>
inline bool LFAST::CommsService::registerMessageHandler(const char *  key, MessageHandler<bool> fn)
{
    this->boolHandlers[key] = fn;
    this->handlerTypes[key] = BOOL_HANDLER;
    return true;
}
template <>
inline bool LFAST::CommsService::registerMessageHandler(const char *  key, MessageHandler<std::string> fn)
{
    this->stringHandlers[key] = fn;
    this->handlerTypes[key] = STRING_HANDLER;
    return true;
}

template <>
inline bool LFAST::CommsService::callMessageHandler(const char *  key, int val)
{
    auto mh = this->intHandlers[key];
    mh.call(val);
    return true;
}

template <>
inline bool LFAST::CommsService::callMessageHandler(const char *  key, unsigned int val)
{
    auto mh = this->uIntHandlers[key];
    mh.call(val);
    return true;
}

template <>
inline bool LFAST::CommsService::callMessageHandler(const char *  key, double val)
{
    auto mh = this->doubleHandlers[key];
    mh.call(val);
    return true;
}

template <>
inline bool LFAST::CommsService::callMessageHandler(const char *  key, bool val)
{
    auto mh = this->boolHandlers[key];
    mh.call(val);
    return true;
}

template <>
inline bool LFAST::CommsService::callMessageHandler(const char *  key, std::string val)
{
    auto mh = this->stringHandlers[key];
    mh.call(val);
    return true;
}

template <>
inline double CommsMessage::getValue(const char *  key)
{
    return (JsonDoc[key].as<double>());
}

template <>
inline int CommsMessage::getValue(const char *  key)
{
    return (JsonDoc[key].as<int>());
}

template <>
inline unsigned int CommsMessage::getValue(const char *  key)
{
    return (JsonDoc[key].as<unsigned int>());
}

template <>
inline bool CommsMessage::getValue(const char *  key)
{
    return (JsonDoc[key].as<bool>());
}

template <>
inline std::string CommsMessage::getValue(const char *  key)
{
    return (std::string(JsonDoc[key].as<const char *>()));
}

inline void CommsMessage::addDestinationKey(const char *  key)
{

    // newDoc.createNestedObject(key);
    // newDoc[key] = JsonDoc.as<JsonObject>();
    destKey = key;

    if (!JsonDoc.isNull())
    {
        StaticJsonDocument<JSON_PROGMEM_SIZE> newDoc;

        for (JsonPairConst kvp : this->JsonDoc.as<JsonObjectConst>())
        {
            newDoc[key][kvp.key()] = kvp.value();
        }
        this->JsonDoc = newDoc;
    }
    else
    {
        JsonDoc.createNestedObject(destKey);
    }

    // = this->getJsonDoc();
    // JsonObject newObj = JsonDoc.
}

// template <typename T>
// inline void CommsMessage::addKeyValuePair(const char *  key, T val)
// {
//     if (this->destKey.length() > 0)
//         JsonDoc[(this->destKey)][key] = val;
//     else
//         JsonDoc[key] = val;
// };

template <typename T>
inline void CommsMessage::addKeyValuePair(const char * key, T val)
{
{
    if (this->destKey.length() > 0)
        JsonDoc[(this->destKey)][(key)] = val;
    else
        JsonDoc[(key)] = val;
};
}

}