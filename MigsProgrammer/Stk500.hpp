/*
 * Author: Dylan Turner
 * Description:
 *  - Implementation of AVR061 STK500 Comm protocol
 *  - Based on:
 *      https://github.com/osbock/Baldwisdom/blob/master/BootDrive/stk500.h
 */

#pragma once

//#define STK500_DEBUG

#include <Arduino.h>

namespace stk500 {
    extern const char *signOnMessage; // Sign on string for GetSignOn

#if defined(STK500_DEBUG)
    extern uint8_t g_resp[];
#endif

    struct AvrMem {
        int size;
        unsigned int pageAddr;
        uint8_t *buff;
    };

    enum class Response {
        Ok = 0x10,
        Failed = 0x11,
        Unknown = 0x12,
        NoDevice = 0x13,
        InSync = 0x14,
        NoSync = 0x15,
        AdcChannelError = 0x16,
        AdcMeasureOk = 0x17,
        PwmChannelError = 0x18,
        PwmAdjustOk = 0x19
    };

    enum class Special {
        CrcEop = 0x20
    };

    enum class Command {
        GetSync = 0x30,
        GetSignOn = 0x31,
        SetParameter = 0x40,
        GetParameter = 0x41,
        SetDevice = 0x42,
        SetDeviceExt = 0x45,
        EnterProgramMode = 0x50,
        LeaveProgramMode = 0x51,
        ChipErase = 0x52,
        CheckAutoInc = 0x53,
        LoadAddress = 0x55,
        Universal = 0x56,
        UniversalMulti = 0x57,
        ProgramFlash = 0x60,
        ProgramData = 0x61,
        ProgramFuse = 0x62,
        ProgramLock = 0x63,
        ProgramPage = 0x64,
        ProgramFuseExt = 0x65,
        ReadFlash = 0x70,
        ReadData = 0x71,
        ReadFuse = 0x72,
        ReadLock = 0x73,
        ReadPage = 0x74,
        ReadSign = 0x75,
        ReadOscCal = 0x76,
        ReadFuseExt = 0x77,
        ReadOscCalExt = 0x78
    };

    enum class Parameter {
        HardwareVersion = 0x80,
        SoftwareMajor = 0x81,
        SoftwareMinor = 0x82,
        Leds = 0x83,
        VTarget = 0x84,
        VAdjust = 0x85,
        OscPScale = 0x86,
        OscCMatch = 0x87,
        ResetDuration = 0x88,
        SckDuration = 0x89,
        BuffSizeL = 0x90,
        BuffSizeH = 0x91,
        Device = 0x92,
        ProgramMode = 0x93,
        ParameterMode = 0x94,
        Polling = 0x95,
        SelfTimed = 0x96,
        TopCardDetect = 0x98
    };

    enum class StatusBit {
        InSync = 0x01,
        ProgramMode = 0x02,
        Standalone = 0x04,
        Reset = 0x08, // Reset button - 1 == pushed
        Program = 0x10, // Program button - 1 == pushed
        LedG = 0x20, // Green status - 1 == lit
        LedR = 0x40, // Red status - 1 == lit
        LedBlink = 0x80 // Blink on/off - 1 == blinking
    };

    enum class Error {
        None = 0,
        Generic = -1,
        UnknownResponse = -2, // stk500_disable(): unknown response
        NoDevice = -3,
        ProtocolSync = -4, // stk500_disable(): protocol error, no Resp::InSync
        NoSync = -5, // Can't get in sync
        NoProgramMode = -6, // Failed to get into programming mode
        NoProgrammer = -7, // Programmer not responding
        NotOk = -8,
        ParameterFailed = -9 // Get parameter failed response
    };

    void send(uint8_t *buff, const unsigned int len);
    Error recv(uint8_t *buff, const unsigned int len);
    void drain(void);
    Error getSync(void);

#if !defined(STK500_DEBUG)
    Error getParam(const Parameter param, unsigned int &ref_val);
    Error loadAddr(const unsigned int addr);
    Error pagedWrite(const AvrMem &mem);
    Error programEnable(void);
    Error disableDevice(void);
#else
    Error getParam(
        const Parameter param, unsigned int &ref_val,
        void (*warning)(Error, const char *),
        void (*error)(Error, const char *)
    );
    Error loadAddr(
        const unsigned int addr, void (*error)(Error, const char *)
    );
    Error pagedWrite(const AvrMem &mem, void (*error)(Error, const char *));
    Error programEnable(void (*error)(Error, const char *));
    Error disableDevice(void (*error)(Error, const char *));
#endif
}
