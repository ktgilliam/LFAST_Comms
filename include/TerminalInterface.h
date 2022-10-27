#pragma once

#include <Arduino.h>
#include <cinttypes>
#include <deque>
#include <string>
#include <vector>
#define CLI_BUFF_LENGTH 90

#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define RESET "\033[0m"

#define TERMINAL_WIDTH 95
#define PRINT_SERVICE_COUNTER 0

namespace LFAST
{
    enum
    {
        INFO = 0,
        DEBUG = 1,
        WARNING = 2,
        ERROR = 3
    };

    enum CLI_HEADER_ROWS
    {
        TOP_HEADER,
        MIDDLE_HEADER,
        LOWER_HEADER,
        EMPTY_1,
        NUM_HEADER_ROWS
    };

    const unsigned int MAX_DEBUG_ROWS = 10;
    const unsigned int MAX_CLOCKBUFF_LEN = 64;
}

class TerminalInterface
{
protected:
    HardwareSerial *serial;
    uint32_t currentInputCol;
    char rxBuff[CLI_BUFF_LENGTH];
    char *rxPtr;
    void handleCliCommand();
    void resetPrompt();
    std::deque<std::string> debugMessages;

    struct PersistentTerminalField
    {
        uint8_t printRow;
        std::string label;
        bool printAsSexa;
    };

private:
    uint16_t debugMessageCount;
    uint16_t firstDebugRow = LFAST::NUM_HEADER_ROWS + 1;
    uint16_t debugRowOffset = 0;
    
    uint16_t promptRow;
    uint16_t messageRow;
    std::string ifLabel;
    uint16_t fieldStartCol = 5;
    std::vector<PersistentTerminalField *> persistentFields;

public:
    TerminalInterface(const std::string &, HardwareSerial *);

    // void updateStatusFields(MountControl &);
    void serviceCLI();
    // void addDebugMessage(std::string&, uint8_t);
    void addDebugMessage(const std::string &msg, uint8_t level = LFAST::INFO);
    // void clearDebugMessages();
    void printHeader();
    void addPersistentField(const std::string &label, uint8_t printRow);

    void updatePersistentField(uint8_t printRow, const std::string &fieldValStr);
    void printPersistentFieldLabels();

    // clang-format off
    inline void clearConsole() {serial->printf("\033[2J"); }
    inline void clearToEndOfRow() { serial->printf("\033[0K"); }
    inline void cursorToRowCol(unsigned int row, unsigned int col) { serial->printf("\033[%u;%uH", row+1, col); }
    inline void cursorToRow(int row) { serial->printf("\033[%u;%uH", (row + 1), 0); }
    inline void cursorToCol(int col) { serial->printf("\033[%uG", col); }

    inline void red() { serial->printf("\033[31m"); }
    inline void green() { serial->printf("\033[32m"); }
    inline void yellow() {serial->printf("\033[33m"); }
    inline void blue() { serial->printf("\033[34m"); }
    inline void magenta() { serial->printf("\033[35m"); }
    inline void cyan() { serial->printf("\033[36m"); }
    inline void white() { serial->printf("\033[37m"); }
    inline void reset() { serial->printf("\033[0m"); }

    inline void blinking() { serial->printf("\033[5m"); }
    inline void notBlinking() { serial->printf("\033[25m"); }
    inline void hideCursor() { serial->printf("\033[?25l"); }
    inline void showCursor() { serial->printf("\033[?25h"); }
    // clang-format on
};

int fs_sexa(char *out, double a, int w, int fracbase);