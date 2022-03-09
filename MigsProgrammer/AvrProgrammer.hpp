/*
 * Author: Dylan Turner
 * Description: Interface for programming another arduino over Serial
 */

#pragma once

//#define PGRMR_DEBUG

#if defined(PGRMR_DEBUG)
#include <SoftwareSerial.h>
#endif
#include <SD.h>
#include "Stk500.hpp"

namespace pgrmr {
    class AvrProgrammer {
        public:
#if defined(PGRMR_DEBUG)
            static void error(const stk500::Error error, const char *msg);
            static void warning(const stk500::Error error, const char *msg);
#endif

            AvrProgrammer(
                const int reset
#if defined(PGRMR_DEBUG)
                , const int errTx, const int errRx,
                const int errBaudRate
#endif
            );

            void init(void);
            void program(File program);
        
        private:
            const int _reset;
            stk500::AvrMem _mem;

#if defined(PGRMR_DEBUG)
            SoftwareSerial _errorSender;
            const int _errBaudRate, _errTx, _errRx;
#endif

            void _toggleReset(void) const;
            int _readPage(const File &program, stk500::AvrMem &ref_mem);
            int _readIntelHexLine(
                File input, int &ref_addr,
                uint8_t *lineMemBuff, uint8_t *lineBuff
            ) const;
            uint8_t _hexToByte(const uint8_t *code) const;
    };
}
