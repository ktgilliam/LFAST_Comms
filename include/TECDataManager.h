#pragma once
#include <cinttypes>
#include <vector>
#include <LFAST_Device.h>
#include <TerminalInterface.h>
#include "ThermoElectricGlobal.h"
// These should be defined in wire.h but they don't appear to be, so creating them here.
#define I2C_NO_ERROR 0
#define I2C_TX_BUFF_OVF 1
#define I2C_RECV_NACK_ON_ADDR_TX 2
#define I2C_RECV_NACK_ON_DATA_TX 3
#define I2C_OTHER_ERROR 4
#define I2C_TIMEOUT 5

#define TEC_COMMAND_SIZE 3
struct TEC_Channel_Config
{
  int dirPin;
  int pwmPin;
  // int thermistorPin; No more thermistor pins, it now goes to an ADC
  bool thermistor;     // is there a thermistor or Seebeck temperature
  int minimum_percent; // the Diodes, inc parts only go from about 15 percent to 100 percent
};

struct TEC_Channel_Data
{
    float dutyCycle;
    float seebeck;
};

typedef enum 
{
    DUTY_COMMAND,
    DUTY_REQUEST,
    SEEBECK_REQUEST,
} TEC_DATA_REQUEST_TYPE;

struct TECDataCommand
{
    TECDataCommand(uint8_t _boardNo=0, uint8_t _channelNo=0, float _value=0.0) : boardNo(_boardNo), channelNo(_channelNo), value(_value), processed(false) {}
    TEC_DATA_REQUEST_TYPE type;
    uint8_t boardNo;
    uint8_t channelNo;
    float value;
    bool processed;
};

typedef enum 
{
    INIT,
    WAITING,
    COLLECT_BOARD_0,
    COLLECT_BOARD_1,
    COLLECT_BOARD_2,
    COLLECT_BOARD_3,
    COLLECT_BOARD_4,
    DONE_COLLECTING
} COLLECTION_STATE;


class TECDataManager : public LFAST_Device
{
public:
    virtual ~TECDataManager() {}
    static TECDataManager &getDeviceController();
    void setupPersistentFields() override;

    void initialize();
    void printStuff(uint8_t board, char *myseebeck);
    int setLocalTECValue(uint8_t tec_chan, double pwmDutyCycle);
    int setRemoteTECValue(uint8_t board, uint8_t tec_chan, double pwmDutyCycle);
    void getRemoteTECSetPoints(uint8_t board);
    void getLocalSeebecks();
    void getRemoteSeebecks(uint8_t board);
    std::vector<TECDataCommand*> requestList;

    void pingCollectionStateMachine();
    // void startCollectionCycle();
    void peripheralMode() { i2cControllerFlag = false; };
    void controllerMode(){ i2cControllerFlag = true; };
    bool isI2cController(){return i2cControllerFlag;}
    void setBoxNo(uint8_t _boxNo){boxNo = _boxNo;}
    void addTecDataCommand(TECDataCommand* rq){requestList.push_back(rq);}

    void processDataCommands();
    void enableControlInterrupt();

private:
    uint8_t i2c_hardware_id;
    TECDataManager(){};
    COLLECTION_STATE currentState;
    void printErrorStatus(uint8_t _error, uint8_t ch, uint8_t board);
    bool i2cControllerFlag;
    uint8_t boxNo;
};

