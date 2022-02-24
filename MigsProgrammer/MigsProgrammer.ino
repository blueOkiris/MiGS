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

const uint32_t g_bootBaud = 115200; // Baud rate for programming over serial
#if defined(PGRMR_DEBUG)
const uint32_t g_errBaud = 19200;
#endif

// Pins
const uint32_t g_chipSelect = 10; // Chip select for SD card
const uint32_t g_reset = 6; // Reset pin for other arduino

const char *g_progName = "blink.hex";

pgrmr::AvrProgrammer g_programmer(
    g_chipSelect, g_reset
#if defined(PGRMR_DEBUG)
    , 4, 5, g_errBaud
#endif
);

void setup(void) {
    Serial.begin(g_bootBaud); // Required for AvrProgrammer to work. Can't be in
    g_programmer.init();
    delay(500);
    g_programmer.program(g_progName);
}

void loop(void) {

}
