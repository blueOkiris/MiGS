/*
 * Author: Dylan Turner
 * Description:
 *  - For Programmer MCU in MiGS
 *  - First, process controller inputs to select a program & program logic mcu
 *  - Second, provide interface to programmed logic mcu for SD Card assets
 *  - Based on:
 *      https://github.com/osbock/Baldwisdom/blob/master/BootDrive/BootDrive.ino
 */

#include "AvrProgrammer.hpp"
#include "ResourceProvider.hpp"

const uint32_t g_bootBaud = 115200; // Baud rate for programming over serial
#if defined(PGRMR_DEBUG)
const uint32_t g_errBaud = 19200;
const char *g_sdInitErrMsg = "Failed to init SD card.";
#endif

// Pins
const uint32_t g_chipSelect = 10; // Chip select for SD card
const uint32_t g_reset = 6; // Reset pin for other arduino

const char *g_progName = "menu.hex";

pgrmr::AvrProgrammer g_programmer(
    g_reset
#if defined(PGRMR_DEBUG)
    , 4, 5, g_errBaud
#endif
);
rsrc::ResourceProvider g_resourceProvider;

void setup(void) {
    Serial.begin(g_bootBaud); // Required for AvrProgrammer to work

    pinMode(g_chipSelect, OUTPUT);
    if(!SD.begin(g_chipSelect)) {
#if defined(PGRMR_DEBUG)
        pgrmr::AvrProgrammer::error(stk500::Error::Generic, g_sdInitErrMsg);
#else
        exit(1);
#endif
    }

    g_programmer.init();
    delay(500);

    // Read hex file from SD card
    if(!SD.exists(g_progName)) {
        while(1);
    }
    File program = SD.open(g_progName, FILE_READ);

    g_programmer.program(program);
}

void loop(void) {
    g_resourceProvider.provide();
}
