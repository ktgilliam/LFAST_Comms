
#include <TerminalInterface.h>

#include <Arduino.h>

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stdio.h>

#include <cinttypes>
#include <mathFuncs.h>
#include <cstring>

// #include <patch.h>

TerminalInterface::TerminalInterface(const std::string &_label, HardwareSerial *_serial)
    : serial(_serial), ifLabel(_label)
{
    clearConsole();
    debugRowOffset = 0;
    debugMessageCount = 0;
    promptRow = LFAST::NUM_HEADER_ROWS + 1;
    printHeader();
    // resetPrompt();
};

void TerminalInterface::printHeader()
{
    cursorToRow(LFAST::TOP_HEADER);
    white();

    std::string HEADER_BORDER_STRING = std::string(TERMINAL_WIDTH, '#');
    std::string HEADER_LABEL_STRING = ifLabel;
    std::string HEADER_LABEL_ROW_SIDE = std::string((TERMINAL_WIDTH - HEADER_LABEL_STRING.size())/2-1, '#');
    std::string HEADER_LABEL_ROW = HEADER_LABEL_ROW_SIDE + " " + HEADER_LABEL_STRING + " " + HEADER_LABEL_ROW_SIDE;

    cursorToRow(LFAST::TOP_HEADER);
    serial->printf("%s", HEADER_BORDER_STRING.c_str());
    cursorToRow(LFAST::MIDDLE_HEADER);
    serial->printf("%s", HEADER_LABEL_ROW.c_str());
    cursorToRow(LFAST::LOWER_HEADER);
    serial->printf("%s", HEADER_BORDER_STRING.c_str());
}

void TerminalInterface::resetPrompt()
{

    messageRow = promptRow + 2;
    cursorToRowCol(messageRow, 0);
    std::string DEBUG_BORDER_STR = std::string(TERMINAL_WIDTH, '-');
    serial->printf("%s", DEBUG_BORDER_STR.c_str());


    showCursor();
    std::memset(rxBuff, '\0', CLI_BUFF_LENGTH);
    rxPtr = rxBuff;
    cursorToRow(promptRow);
    serial->print(">> ");
    clearToEndOfRow();
    currentInputCol = 4;
    cursorToCol(currentInputCol);



    // BLINKING();
}

void TerminalInterface::serviceCLI()
{
#if PRINT_SERVICE_COUNTER
    static uint64_t serviceCounter = 0;
    cursorToRowCol(SERVICE_COUNTER_ROW, 0);
    serial->printf("[%o]", serviceCounter++);
#endif
    // static int64_t cnt =0;
    cursorToRowCol(promptRow, currentInputCol);
    if (serial->available() > 0)
    {
        // read the incoming byte:
        char c = serial->read();

        if (c == '\r' || c == '\n')
        {
            handleCliCommand();
        }
        else
        {
            // say what you got:
            serial->printf("%c", c);
            // Put it in the buffer
            if (rxPtr < (rxBuff + CLI_BUFF_LENGTH - 1))
            {
                *rxPtr++ = c;
                currentInputCol++;
            }
        }
        // cnt = 0;
    }
    // else
    // {
    // serial->printf("%d", cnt++);
    // }

}

void TerminalInterface::handleCliCommand()
{
    cursorToRow(promptRow + 1);
    clearToEndOfRow();
    serial->printf("%s: Command Not Found.\r\n", rxBuff);

    resetPrompt();
}

void TerminalInterface::addPersistentField(const std::string &label, uint8_t printRow)
{
    static uint16_t highestFieldRowNum = 0;
    uint16_t adjustedPrintRow = printRow + LFAST::NUM_HEADER_ROWS;
    if (adjustedPrintRow > highestFieldRowNum)
    {
        highestFieldRowNum = adjustedPrintRow;
        promptRow = highestFieldRowNum + 3;
        firstDebugRow = promptRow + 3;
    }
    if (fieldStartCol < (label.size() + 1))
    {
        fieldStartCol = label.size() + 1;
    }
    PersistentTerminalField *field = new PersistentTerminalField();
    field->printRow = adjustedPrintRow;
    field->label = label;
    persistentFields.push_back(field);
    // resetPrompt();
}

void TerminalInterface::printPersistentFieldLabels()
{
    for (auto field : persistentFields)
    {
        cursorToRow(field->printRow);
        clearToEndOfRow();
        serial->printf("\033[0K\033[37m%s:\033[22G", field->label.c_str());
    }

    // cursorToRowCol(DEBUG_BORDER_1, 0);
    // std::string DEBUG_BORDER_STR = std::string(TERMINAL_WIDTH, '-');
    // serial->printf("%s", DEBUG_BORDER_STR.c_str());

    resetPrompt();
}

void TerminalInterface::addDebugMessage(const std::string &msg, uint8_t level)
{
    debugMessageCount++;
    std::string colorStr;
    switch (level)
    {
    case LFAST::INFO:
        colorStr = WHITE;
        break;
    case LFAST::DEBUG:
        colorStr = GREEN;
        break;
    case LFAST::WARNING:
        colorStr = YELLOW;
        break;
    case LFAST::ERROR:
        colorStr = RED;
        break;
    }
    std::stringstream ss;
    ss << std::setiosflags(std::ios::left) << std::setw(6);
    ss << WHITE << debugMessageCount << ": " << colorStr << msg;
    std::string msgPrintSr = ss.str();

    // serial->printf("[%d]", debugMessageCount);

    if (debugMessages.size() < LFAST::MAX_DEBUG_ROWS)
    {
        cursorToRowCol((firstDebugRow+debugRowOffset++), 0);
        clearToEndOfRow();
        debugMessages.push_back(msgPrintSr);
        serial->println(msgPrintSr.c_str());
    }
    else
    {
        debugMessages.pop_front();
        debugMessages.push_back(msgPrintSr);
        for (uint16_t ii = 0; ii < LFAST::MAX_DEBUG_ROWS; ii++)
        {
            cursorToRowCol((firstDebugRow + ii), 0);
            clearToEndOfRow();
            serial->print(debugMessages.at(ii).c_str());
            // serial->printf(" [fdr:%d][dro:%d][ii:%d]", firstDebugRow, debugRowOffset, ii);
        }
    }
}

void TerminalInterface::updatePersistentField(uint8_t printRow, const std::string &fieldValStr)
{
    hideCursor();
    uint16_t adjustedPrintRow = printRow + LFAST::NUM_HEADER_ROWS;
    cursorToRowCol(adjustedPrintRow, fieldStartCol+4);
    // clearToEndOfRow();
    // cursorTCol(fieldStartCol+4);
    serial->print(fieldValStr.c_str());
    clearToEndOfRow();
    // showCursor();
}

int fs_sexa(char *out, double a, int w, int fracbase)
{
    char *out0 = out;
    unsigned long n;
    int d;
    int f;
    int m;
    int s;
    int isneg;

    /* save whether it's negative but do all the rest with a positive */
    isneg = (a < 0);
    if (isneg)
        a = -a;

    /* convert to an integral number of whole portions */
    n = (unsigned long)(a * fracbase + 0.5);
    d = n / fracbase;
    f = n % fracbase;

    /* form the whole part; "negative 0" is a special case */
    if (isneg && d == 0)
        out += snprintf(out, LFAST::MAX_CLOCKBUFF_LEN, "%*s-0", w - 2, "");
    else
        out += snprintf(out, LFAST::MAX_CLOCKBUFF_LEN, "%*d", w, isneg ? -d : d);

    /* do the rest */
    switch (fracbase)
    {
    case 60: /* dd:mm */
        m = f / (fracbase / 60);
        out += snprintf(out, LFAST::MAX_CLOCKBUFF_LEN, ":%02d", m);
        break;
    case 600: /* dd:mm.m */
        out += snprintf(out, LFAST::MAX_CLOCKBUFF_LEN, ":%02d.%1d", f / 10, f % 10);
        break;
    case 3600: /* dd:mm:ss */
        m = f / (fracbase / 60);
        s = f % (fracbase / 60);
        out += snprintf(out, LFAST::MAX_CLOCKBUFF_LEN, ":%02d:%02d", m, s);
        break;
    case 36000: /* dd:mm:ss.s*/
        m = f / (fracbase / 60);
        s = f % (fracbase / 60);
        out += snprintf(out, LFAST::MAX_CLOCKBUFF_LEN, ":%02d:%02d.%1d", m, s / 10, s % 10);
        break;
    case 360000: /* dd:mm:ss.ss */
        m = f / (fracbase / 60);
        s = f % (fracbase / 60);
        out += snprintf(out, LFAST::MAX_CLOCKBUFF_LEN, ":%02d:%02d.%02d", m, s / 100, s % 100);
        break;
    default:
        printf("fs_sexa: unknown fracbase: %d\n", fracbase);
        return -1;
    }

    return (out - out0);
}
