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
/// @file TerminalInterface.h
///
/// The terminal interface uses a teensy serial port to display data and report
/// messages in an organized way. 
/// - To set which port, edit the TEST_SERIAL_NO field in platformio.ini
/// - To set the baud rate, edit the TEST_SERIAL_BAUD in platformio.ini

#pragma once
#include <Arduino.h>
#include <cinttypes>
#include <deque>
#include <string>
#include <vector>
#include <map>
#include <cstdio>

#include <teensy41_device.h>

// #if defined(TERMINAL_ENABLED)
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
    /// @brief Effects the color the message prints in.
    enum
    {
        INFO_MESSAGE = 0,
        DEBUG_MESSAGE = 1,
        WARNING_MESSAGE = 2,
        ERROR_MESSAGE = 3
    };

    enum CLI_HEADER_ROWS
    {
        TOP_HEADER,
        MIDDLE_HEADER,
        LOWER_HEADER,
        EMPTY_1,
        NUM_HEADER_ROWS
    };

    const unsigned int MAX_DEBUG_ROWS = 15;
    const unsigned int MAX_CLOCKBUFF_LEN = 64;
}

class TerminalInterface
{
protected:
    TEST_SERIAL_TYPE *serial;

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

    void initialize();
private:
    uint16_t debugMessageCount;
    uint16_t firstDebugRow = LFAST::NUM_HEADER_ROWS + 1;
    uint16_t debugRowOffset = 0;

    uint16_t promptRow;
    uint16_t messageRow;
    std::string ifLabel;
    uint16_t fieldStartCol = 5;
    std::vector<PersistentTerminalField *> persistentFields;
    std::map<std::string, uint8_t> senderRowOffsetMap;
    uint16_t highestFieldRowNum;

public:
    TerminalInterface(const std::string &, TEST_SERIAL_TYPE *, uint32_t);

    void registerDevice(const std::string &);
    void serviceCLI();
    template <typename... Args>
    void printfDebugMessage(const char *fmt, Args... args);
    void printDebugMessage(const std::string &msg, uint8_t level = LFAST::INFO_MESSAGE);

    void printHeader();
    void addPersistentField(const std::string &device, const std::string &label, uint8_t printRow);
    void updatePersistentField(const std::string &device, uint8_t printRow, int fieldVal);
    void updatePersistentField(const std::string &device, uint8_t printRow, long fieldVal);
    void updatePersistentField(const std::string &device, uint8_t printRow, const std::string &fieldValStr);
    void updatePersistentField(const std::string &device, uint8_t printRow, double fieldVal, const char *fmt = "%6.4f");

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
    // void printDebugInfo();
};

/// @brief Print a debug message using printf-style formatting
/// @param fmt Format string
/// @param args Values to print in the formatted string
template <typename... Args>
void TerminalInterface::printfDebugMessage(const char *fmt, Args... args)
{
    char msgBuff[100]{0};
    sprintf(msgBuff, fmt, args...);
    printDebugMessage(msgBuff);
}

int fs_sexa(char *out, double a, int w, int fracbase);

#include <sstream>
inline std::string debugCodeIdStr(std::string file, int line)
{

    size_t pos = 0;
    std::string token;
    std::string delimiter = "/";
    while ((pos = file.find(delimiter)) != std::string::npos)
    {
        token = file.substr(0, pos);
        file.erase(0, pos + delimiter.length());
    }

    std::stringstream ss;
    ss << file << "[" << line << "]";
    return ss.str();
}

#define DEBUG_CODE_ID_STR debugCodeIdStr(__FILE__, __LINE__)

// #endif