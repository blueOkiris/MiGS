/*
 * Author: Dylan Turner
 * Description:
 * - First program programmed to MiGS Logic MCU
 * - Receive list of programs and draw to screen
 * - Follow inputs to appear like you are selecting one
 */

#include <Wire.h>
#include "font.hpp"

const int g_i2cAddr = 0x7C;
const int g_numDispGames = 10;
const uint16_t g_textXOffset = 14;
const uint8_t g_textYOffset = 4;
const uint8_t g_textYSpacing = 2;
const int g_fNameLenLimit = 13; // 8.3

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
bool g_updateListText = false;

void setup(void) {
    // Set up communication with the programmer/resource getter
    Serial.begin(115200);
    loadGameList();

    // Set up communication to the GPU
    Wire.begin(g_i2cAddr);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
}

// We're mostly interested in sending data TO the GPU, so this doesn't do much
void receiveEvent(int bytesRecv) {
    while(Wire.available()) {
        Wire.read();
    }
}

void requestEvent() {
    if(g_updateListText) {
        for(int i = 0; i < g_numDispGames; i++) {
            Wire.write('T');
            Wire.write((uint8_t) ((g_textXOffset >> 8) & 0xFF));
            Wire.write((uint8_t) (g_textXOffset & 0xFF));
            Wire.write(g_textYOffset + i * g_textYSpacing);
            for(int j = 0; j < g_fNameLenLimit; j++) {
                Wire.write(g_gameList[j]);
            }
        }
        g_updateListText = false;
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
