/*
 * Author: Dylan Turner
 * Description: Implementation of resource provider class
 */

#include <Arduino.h>
#include "ResourceProvider.hpp"

using namespace rsrc;

ResourceProvider::ResourceProvider(void) {
}

void ResourceProvider::provide(void) {
    bool quit = false;
    while(!quit) {
        if(Serial.available()) {
            uint8_t cmd = Serial.read();
            switch(cmd) {
                case 0x55:
                    quit = true;
                    break;

                case 'F':
                    // TODO: Read a file and send its contents over serial
                    break;

                case 'L':
                    // TODO: Send list of games over serial
                    break;
            }
        }
    }
}
