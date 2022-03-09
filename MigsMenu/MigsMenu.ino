/*
 * Author: Dylan Turner
 * Description:
 *  - First program programmed to MiGS Logic MCU
 *  - Receive list of programs and draw to screen
 *  - Follow inputs to appear like you are selecting one
 */

#include <Wire.h>

#define I2C_ADDR    0x7C

void receiveEvent(int bytes_recv) {
    while(Wire.available()) {
        char a = Wire.read();
        Serial.print(F("Received"));
        Serial.println(a);
    }
}

void requestEvent() {
    Wire.write(0xFF);
    Serial.println(F("Sent!"));
}

void setup(void) {
    Serial.begin(115200);
    Wire.begin(I2C_ADDR);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
    Serial.println(F("Starting!"));
}

void loop(void) {
    delay(100);
}
