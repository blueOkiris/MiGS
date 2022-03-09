/*
 * Author: Dylan Turner
 * Description: Implementation of resource provider class
 */

#include <Arduino.h>
#include <SD.h>
#include "ResourceProvider.hpp"

using namespace rsrc;

// Save on dynamic resource breaking system by using globals
char g_fName[13]; // Only allow root dir 8.3 filenames + \0 for now
int g_fNameInd = 0;
File g_program, g_entry;

ResourceProvider::ResourceProvider(void) {
}

void ResourceProvider::provide(void) {
    while(true) {
        if(Serial.available()) {
            uint8_t cmd = Serial.read();
            switch(cmd) {
                case 0x55:
                    return;

                case 'F':
                    g_fNameInd = 0;
                    while(Serial.available()) {
                        g_fName[g_fNameInd++] = Serial.read();
                    }
                    g_fName[g_fNameInd] = 0;
                    if(!SD.exists(g_fName)) {
                        while(1);
                    }
                    g_program = SD.open(g_fName, FILE_READ);
                    while(g_program.available()) {
                        Serial.write(g_program.read());
                    }
                    g_program.close();
                    break;

                case 'L':
                    g_program = SD.open("/", FILE_READ);
                    while(true) {
                        g_entry = g_program.openNextFile();
                        if(!g_entry) {
                            break;
                        }

                        for(g_fNameInd = 0;
                                g_fNameInd < strlen(g_entry.name());
                                g_fNameInd++) {
                            Serial.write(g_entry.name()[g_fNameInd]);
                        }
                        g_entry.close();
                    }
                    break;
            }
        }
    }
}
