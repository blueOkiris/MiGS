/*
 * Author: Dylan Turner
 * Description:
 * - First program programmed to MiGS Logic MCU
 * - Receive list of programs and draw to screen
 * - Follow inputs to appear like you are selecting one
 */

#include <Wire.h>
#include "Font.hpp"

const int g_i2cAddr = 0x7C;
const int g_numDispGames = 10;
const uint16_t g_textXOffset = 14;
const uint8_t g_textYOffset = 4;
const uint8_t g_textYSpacing = 2;
const int g_fNameLenLimit = 13; // 8.3
const uint16_t g_bg = 0x07FF;

int g_listInd = 0;
char g_gameList[g_numDispGames][g_fNameLenLimit] = {
    "            ",
    "            ",
    "            ",
    "            ",
    "            ",
    "            ",
    "            ",
    "            ",
    "            ",
    "            "
};

// Flags for sending updated data
int g_fontsLoaded = 0;
bool g_tempSendA = true;
bool g_updateListText = false;
bool g_setBg = true;

// Data to send
uint8_t g_sendBuff[128];

void setup(void) {
    // Set up communication with the programmer/resource getter
    Serial.begin(115200);
    //loadGameList();

    // Set up communication to the GPU
    Wire.begin(g_i2cAddr);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);

    for(int i = 0; i < 129; i++) {
        g_sendBuff[i] = 0;
    }
    g_sendBuff[0] = 0x55;
}

// We're mostly interested in sending data TO the GPU, so this doesn't do much
void receiveEvent(int bytesRecv) {
    while(Wire.available()) {
        Wire.read();
    }
}

void requestEvent() {
    if(g_setBg) {
        Wire.write('B');
        Wire.write((uint8_t) ((g_bg >> 8) & 0xFF));
        Wire.write((uint8_t) (g_bg & 0xFF));
        g_setBg = false;
    } else if(g_fontsLoaded < 2) {
        Wire.write('D');
        for(int i = 0; i < 128; i++) {
            g_sendBuff[i] = pgm_read_byte_near(
                font::g_fontSprs[g_fontsLoaded] + i
            );
        }
        Wire.write(g_sendBuff, 128);
        g_fontsLoaded++;
    } else if (g_tempSendA) {
        Wire.write('S');

        Wire.write(0);
        Wire.write(13);

        Wire.write(0);
        Wire.write(27);

        Wire.write(0);
        Wire.write(FONT_CAP_START);

        g_tempSendA = false;
    } else {
        Wire.write(0x55); // Send "0x55" to the GPU to indicate "do nothing"
    }
}

// Reach out to resource provider and ask for a list of games
void loadGameList(void) {
    Serial.write('L');
    int gameInd = 0, offset = g_listInd;
    while(!Serial.available());
    while(Serial.available()) {
        char cmd = Serial.read();
        if(cmd == 'F') {
            if(gameInd < g_numDispGames) { // Just load in and we'll shift later
                for(int i = 0; i < g_fNameLenLimit; i++) {
                    g_gameList[gameInd][i++] = Serial.read();
                }
                gameInd++;
            } else if(offset > 0) { // We have a 1st that isn't actually the 1st
                // Shift over and lose the first ("scroll")
                for(int i = 1; i < g_numDispGames; i++) {
                    for(int j = 0; j < g_fNameLenLimit; j++) {
                        g_gameList[i - 1][j] = g_gameList[i][j];
                    }
                }

                // Fill in new bottom
                for(int i = 0; i < g_fNameLenLimit; i++) {
                    g_gameList[g_numDispGames - 1][i++] = Serial.read();
                }
                offset--;
            } else {
                break; // Ignore any past offset + 10
            }
        }
    }
    g_updateListText = true;
}

void loop(void) {
    delay(100);
}
