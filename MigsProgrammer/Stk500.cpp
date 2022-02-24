/*
 * Author: Dylan Turner
 * Description:
 *  - Const data and function location for Stk500.hpp
 *  - Based on
 *      https://github.com/osbock/Baldwisdom/blob/master/BootDrive/BootDrive.ino
 */

#include <Arduino.h>
#include "Stk500.hpp"

// Use lots of globals here in order to conserve limited RAM

const char *stk500::signOnMessage = "AVR STK";

#if !defined(STK500_DEBUG)
uint8_t g_msg[8], g_resp[8];
#else
uint8_t g_msg[8], stk500::g_resp[8];
#endif

const int g_memType = 'F';
const int g_pageSize = 128;
const int g_flash = 1;

int g_tries = 0;
bool g_quit = false;

#if defined(STK500_DEBUG)
const char *g_unrecoverable1ErrMsg = "Unrecoverable #1";
const char *g_unrecoverable2ErrMsg = "Unrecoverable #2";
const char *g_unrecoverable3ErrMsg = "Unrecoverable #3";
#endif

void stk500::send(uint8_t *buff, const unsigned int len) {
    Serial.write(buff, len);
}

stk500::Error stk500::recv(uint8_t *buff, const unsigned int len) {
    if(Serial.readBytes(buff, len) < 0) {
        return Error::NoProgrammer;
    }
    return Error::None;
}

void stk500::drain(void) {
    while(Serial.available() > 0) {
        Serial.read();
    }
}

stk500::Error stk500::getSync(void) {
    // Get in sync
    g_msg[0] = static_cast<uint8_t>(Command::GetSync);
    g_msg[1] = static_cast<uint8_t>(Special::CrcEop);

    // First send and drain a few times to get rid of noise
    send(g_msg, 2);
    drain();
    send(g_msg, 2);
    drain();

    send(g_msg, 2);
    if(recv(g_resp, 1) != Error::None) {
        return Error::Generic;
    }
    if(g_resp[0] != static_cast<uint8_t>(Response::InSync)) {
        drain();
        return Error::ProtocolSync;
    }

    if(recv(g_resp, 1) != Error::None) {
        return Error::Generic;
    }
    if(g_resp[0] != static_cast<uint8_t>(Response::Ok)) {
        return Error::NotOk;
    }

    return Error::None;
}

#if !defined(STK500_DEBUG)
stk500::Error stk500::getParam(const Parameter param, unsigned int &ref_val) {
#else
stk500::Error stk500::getParam(
        const Parameter param, unsigned int &ref_val,
        void (*warning)(Error, const char *),
        void (*error)(Error, const char *)) {
#endif
    unsigned int val;

    g_tries = 0;
    g_quit = false;
    while(!g_quit) {
        g_tries++;

        // Reset to get param command
        g_msg[0] = static_cast<uint8_t>(Command::GetParameter);
        g_msg[1] = static_cast<uint8_t>(param);
        g_msg[2] = static_cast<uint8_t>(Special::CrcEop);
        send(g_msg, 3);

        if(recv(g_resp, 1) != Error::None) {
#if defined(STK500_DEBUG)
            error(Error::Generic, g_unrecoverable1ErrMsg);
#else
            exit(1);
#endif
        }
        if(g_resp[0] == static_cast<uint8_t>(Response::NoSync)) {
            if(g_tries > 33) {
#if defined(STK500_DEBUG)
                warning(Error::NoSync, "Get param too many retries.");
#endif
                return Error::NoSync;
            }
            Error syncErr = getSync(); // Modifies g_msg too, but non-issue
            if(syncErr != Error::None) {
#if defined(STK500_DEBUG)
                warning(Error::NoSync, "Get param sync failed.");
#endif
                return syncErr;
            }
            continue;
        }
        if(g_resp[0] != static_cast<uint8_t>(Response::InSync)) {
#if defined(STK500_DEBUG)
            warning(Error::ProtocolSync, "Get param not in sync.");
#endif
            return Error::ProtocolSync;
        }

        g_quit = true;
    }

    if(recv(g_resp, 1) != Error::None) {
#if defined(STK500_DEBUG)
        error(Error::Generic, g_unrecoverable2ErrMsg);
#else
        exit(1);
#endif
    }
    val = g_resp[0];

    if(recv(g_resp, 1) != Error::None) {
#if defined(STK500_DEBUG)
        error(Error::Generic, g_unrecoverable3ErrMsg);
#else
        exit(1);
#endif
    }
    if(g_resp[0] == static_cast<uint8_t>(Response::Failed)) {
        return Error::ParameterFailed;
    }
    if(g_resp[0] != static_cast<uint8_t>(Response::Ok)) {
        return Error::NotOk;
    }

    ref_val = val;
    return Error::None;
}

#if !defined(STK500_DEBUG)
stk500::Error stk500::loadAddr(const unsigned int addr) {
#else
stk500::Error stk500::loadAddr(
        const unsigned int addr, void (*error)(Error, const char *)) {
#endif
    g_tries = 0;
    g_quit = false;
    while(!g_quit) {
        g_tries++;

        // Reset load command
        g_msg[0] = static_cast<uint8_t>(Command::LoadAddress);
        g_msg[1] = addr & 0xFF;
        g_msg[2] = (addr >> 8) & 0xFF;
        g_msg[3] = static_cast<uint8_t>(Special::CrcEop);
        send(g_msg, 4);

        if(recv(g_resp, 1) != Error::None) {
#if defined(STK500_DEBUG)
            error(Error::Generic, g_unrecoverable1ErrMsg);
#else
            exit(1);
#endif
        }
        if(g_resp[0] == static_cast<uint8_t>(Response::NoSync)) {
            if(g_tries > 33) {
                return Error::NoSync;
            }
            Error syncErr = getSync(); // Modifies g_msg, but non-issue
            if(syncErr != Error::None) {
                return syncErr;
            }
            continue;
        }
        if(g_resp[0] != static_cast<uint8_t>(Response::InSync)) {
            return Error::ProtocolSync;
        }

        g_quit = true;
    }

    if(recv(g_resp, 1) != Error::None) {
#if defined(STK500_DEBUG)
        error(Error::Generic, g_unrecoverable2ErrMsg);
#else
        exit(1);
#endif
    }
    if(g_resp[0] == static_cast<uint8_t>(Response::Ok)) {
        return Error::None;
    }

    return Error::NotOk;
}

// NOTE: Eeprom not supported
#if !defined(STK500_DEBUG)
stk500::Error stk500::pagedWrite(const AvrMem &mem) {
#else
stk500::Error stk500::pagedWrite(
        const AvrMem &mem, void (*error)(Error, const char *)) {
#endif
    // Send data separately on arduino
    g_msg[0] = static_cast<uint8_t>(Command::ProgramPage);
    g_msg[1] = (g_pageSize >> 8) & 0xFF;
    g_msg[2] = g_pageSize & 0xFF;
    g_msg[3] = g_memType;
    send(g_msg, 4);
    send(&mem.buff[0], g_pageSize);
    g_msg[0] = static_cast<uint8_t>(Special::CrcEop);
    send(g_msg, 1);

    if(recv(g_resp, 1) != Error::None) {
#if defined(STK500_DEBUG)
        error(Error::Generic, g_unrecoverable1ErrMsg);
#else
        exit(1);
#endif
    }
    if(g_resp[0] == static_cast<uint8_t>(Response::NoSync)) {
        return Error::NoSync;
    } 
    if(g_resp[0] != static_cast<uint8_t>(Response::InSync)) {
        return Error::ProtocolSync;
    }

    if(recv(g_resp, 1) != Error::None) {
#if defined(STK500_DEBUG)
        error(Error::Generic, g_unrecoverable2ErrMsg);
#else
        exit(1);
#endif
    }
    if(g_resp[0] != static_cast<uint8_t>(Response::Ok)) {
        return Error::NotOk;
    }

    return Error::None;
}

#if !defined(STK500_DEBUG)
stk500::Error stk500::programEnable(void) {
#else
stk500::Error stk500::programEnable(void (*error)(Error, const char *)) {
#endif
    g_tries = 0;
    g_quit = false;
    while(!g_quit) {
        g_tries++;

        g_msg[0] = static_cast<uint8_t>(Command::EnterProgramMode);
        g_msg[1] = static_cast<uint8_t>(Special::CrcEop);
        send(g_msg, 2);

        if(recv(g_resp, 1) != Error::None) {
#if defined(STK500_DEBUG)
            error(Error::Generic, g_unrecoverable1ErrMsg);
#else
            exit(1);
#endif
        }
        if(g_resp[0] == static_cast<uint8_t>(Response::NoSync)) {
            if(g_tries > 33) {
                return Error::NoSync;
            }
            Error syncErr = getSync(); // modifies g_msg, but non-issue
            if(syncErr != Error::None) {
                return syncErr;
            }
            continue;
        }
        if(g_resp[0] != static_cast<uint8_t>(Response::InSync)) {
            return Error::ProtocolSync;
        }

        g_quit = true;
    }

    if(recv(g_resp, 1) != Error::None) {
#if defined(STK500_DEBUG)
        error(Error::Generic, g_unrecoverable2ErrMsg);
#else
        exit(1);
#endif
    }
    if(g_resp[0] == static_cast<uint8_t>(Response::Ok)) {
        return Error::None;
    }
    if(g_resp[0] == static_cast<uint8_t>(Response::NoDevice)) {
        return Error::NoDevice;
    }
    if(g_resp[0] == static_cast<uint8_t>(Response::Failed)) {
        return Error::NoProgramMode;
    }

    return Error::UnknownResponse;
}

#if !defined(STK500_DEBUG)
stk500::Error stk500::disableDevice(void) {
#else
stk500::Error stk500::disableDevice(void (*error)(Error, const char *)) {
#endif
    g_tries = 0;
    g_quit = false;
    while(!g_quit) {
        g_tries++;

        g_msg[0] = static_cast<uint8_t>(Command::LeaveProgramMode);
        g_msg[1] = static_cast<uint8_t>(Special::CrcEop);
        send(g_msg, 2);

        if(recv(g_resp, 1) != Error::None) {
#if defined(STK500_DEBUG)
            error(Error::Generic, g_unrecoverable1ErrMsg);
#else
            exit(1);
#endif
        }
        if(g_resp[0] == static_cast<uint8_t>(Response::NoSync)) {
            if(g_tries > 33) {
                return Error::NoSync;
            }
            Error syncErr = getSync();
            if(syncErr != Error::None) {
                return syncErr;
            }
            continue;
        }
        if(g_resp[0] != static_cast<uint8_t>(Response::InSync)) {
            return Error::ProtocolSync;
        }

        g_quit = true;
    }

    if(recv(g_resp, 1) != Error::None) {
#if defined(STK500_DEBUG)
        error(Error::Generic, g_unrecoverable2ErrMsg);
#else
        exit(1);
#endif
    }
    if(g_resp[0] == static_cast<uint8_t>(Response::Ok)) {
        return Error::None;
    }
    if(g_resp[0] == static_cast<uint8_t>(Response::NoDevice)) {
        return Error::NoDevice;
    }

    return Error::UnknownResponse;
}
