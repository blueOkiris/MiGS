/*
 * Author: Dylan Turner
 * Description: Implementation of programmer
 */

#include <SD.h>
#include "Stk500.hpp"
#include "AvrProgrammer.hpp"
#if defined(PGRMR_DEBUG)
#include <SoftwareSerial.h>
#endif

using namespace pgrmr;

uint8_t g_memPage[128];

// Used for reading a page
uint8_t g_lineMemBuff[16];
uint8_t g_lineBuff[50];

#if defined(PGRMR_DEBUG)
// Error messages (for space)
const char *g_syncErrMsg = "Problem getting in sync.";
const char *g_sdFileNotFoundErrMsg = "Failed while opening file.";
const char *g_pgrmModeErrMsg = "Problem entering program mode.";
const char *g_loadAddrErrMsg = "Problem while loading page address.";
const char *g_pagedWriteErrMsg = "Problem while writing page.";
const char *g_disableErrMsg = "Problem disabling device.";

// Keep track of error throughout programArduino
stk500::Error g_err = stk500::Error::None;

SoftwareSerial *g_errorSender = nullptr;
#endif

#if !defined(PGRMR_DEBUG)
AvrProgrammer::AvrProgrammer(const int reset) : _reset(reset) {
}
#else
AvrProgrammer::AvrProgrammer(
        const int reset,
        const int errTx, const int errRx, const int errBaudRate) :
        _reset(reset),
        _errorSender(errRx, errTx),
        _errTx(errTx), _errRx(errRx), _errBaudRate(errBaudRate) {
}
#endif

void AvrProgrammer::init(void) {
#if defined(PGRMR_DEBUG)
    pinMode(_errRx, INPUT);
    pinMode(_errTx, OUTPUT);
    _errorSender.begin(_errBaudRate);
    g_errorSender = &_errorSender;
#endif
    pinMode(_reset, OUTPUT);
    digitalWrite(_reset, HIGH);

    _mem.buff = g_memPage;

#if defined(PGRMR_DEBUG)
    _errorSender.println(F("SD Card initialized."));
#endif
}

void AvrProgrammer::program(File program) {
    // Reset other arduino, get in sync, and get its software info
    digitalWrite(_reset, HIGH);
    delay(100);
    _toggleReset();
    delay(10);

#if !defined(PGRMR_DEBUG)
    stk500::getSync();

    stk500::programEnable();
    while(_readPage(program, _mem)) {
        stk500::loadAddr(_mem.pageAddr >> 1);
        stk500::pagedWrite(_mem);
    }

    // Close out
    stk500::disableDevice();
    delay(10);
    _toggleReset();
    program.close();
#else
    g_err = stk500::getSync();
    if(g_err != stk500::Error::None) {
        warning(g_err, g_syncErrMsg);
    }

    // Program arduino
    g_err = stk500::programEnable(error);
    if(g_err != stk500::Error::None) {
        warning(g_err, g_pgrmModeErrMsg);
    }
    _errorSender.println(F("Entered program mode."));
    while(_readPage(program, _mem)) {
        g_err = stk500::loadAddr(_mem.pageAddr >> 1, error);
        if(g_err != stk500::Error::None) {
            warning(g_err, g_loadAddrErrMsg);
        }
        g_err = stk500::pagedWrite(_mem, error);
        if(g_err != stk500::Error::None) {
            warning(g_err, g_pagedWriteErrMsg);
        }
    }
    _errorSender.println(F("Finished programming."));

    // Close out
    g_err = stk500::disableDevice(error);
    if(g_err != stk500::Error::None) {
        warning(g_err, g_disableErrMsg);
    }
    delay(10);
    _toggleReset();
    program.close();
    _errorSender.println(F("Done."));
#endif
}

void AvrProgrammer::_toggleReset(void) const {
    digitalWrite(_reset, LOW);
    delayMicroseconds(1000);
    digitalWrite(_reset, HIGH);
}

int AvrProgrammer::_readPage(const File &program, stk500::AvrMem &ref_mem) {
    // Grab 128 bytes or less (i.e. a page)
    int totalLen = 0;
    for(int i = 0; i < 8; i++) {
        int addr;
        int len = _readIntelHexLine(program, addr, g_lineMemBuff, g_lineBuff);
        if(len < 0) {
            break;
        } else {
            totalLen += len;
        }
        if(i == 0) { // first record determines page address
            ref_mem.pageAddr = addr;
        }
        memcpy((ref_mem.buff) + (i * 16), g_lineMemBuff, len);
    }
    ref_mem.size = totalLen;
    return totalLen;
}

// :<8-bit record size><16bit address><8bit record type><data...><8bit checksum>
int AvrProgrammer::_readIntelHexLine(
        File input, int &ref_addr,
        uint8_t *lineMemBuff, uint8_t *lineBuff) const {
    uint8_t c;
    int lineBuffInd = 0;
    while(true) {
        if(input.available()) {
            c = input.read();
#if defined(PGRMR_DEBUG)
            _errorSender.print((char) c);
#endif
            if((c == '\n') || (c == '\r')) {
                break;
            } else {
                lineBuff[lineBuffInd++] = c;
            }
        } else {
            return -1;
        }
    }

    lineBuff[lineBuffInd] = 0; // Terminate string

    if(input.peek() == '\n') {
        input.read();
    }

    int len = _hexToByte(&lineBuff[1]);
    ref_addr = (_hexToByte(&lineBuff[3]) << 8) | _hexToByte(&lineBuff[5]);
    for(int i = 9, j = 0; i < ((len * 2) + 9); i += 2, j++) {
        lineMemBuff[j] = _hexToByte(&lineBuff[i]);
    }

    return len;
}

uint8_t AvrProgrammer::_hexToByte(const uint8_t *code) const {
    uint8_t result = 0;
    if ((code[0] >= '0') && (code[0] <= '9')) {
        result = ((int) code[0] - '0') << 4;
    } else if ((code[0] >= 'A') && (code[0] <= 'F')) {
        result = ((int) code[0] - 'A' + 10) << 4;
    }
    if ((code[1] >= '0') && (code[1] <= '9')) {
        result |= ((int) code[1] - '0');
    } else if ((code[1] >= 'A') && (code[1] <= 'F')) {
        result |= ((int) code[1] -'A' + 10);
    }
    return result;
}

#if defined(PGRMR_DEBUG)
void AvrProgrammer::error(const stk500::Error error, const char *msg) {
    int errInd = -static_cast<int>(error);
    g_errorSender->print(F("Error: "));
    g_errorSender->print(msg);
    g_errorSender->print(F(" Code: "));
    g_errorSender->print(static_cast<int>(error), DEC);
    g_errorSender->println(F("."));
    while(true);
}

void AvrProgrammer::warning(const stk500::Error error, const char *msg) {
    int errInd = -static_cast<int>(error);
    g_errorSender->print(F("Warning: "));
    g_errorSender->print(msg);
    g_errorSender->print(F(" Code: "));
    g_errorSender->print(static_cast<int>(error), DEC);
    g_errorSender->println(F("."));
}
#endif
