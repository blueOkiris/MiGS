/*
 * Author: Dylan Turner
 * Description: Receive error messages from the pgrmr
 */

#include <SoftwareSerial.h>

SoftwareSerial errorReceiver(5, 4); // rx, tx

void setup() {
    Serial.begin(115200);
    errorReceiver.begin(19200);
}

void loop() {
    while(errorReceiver.available()) {
        Serial.write(errorReceiver.read());
    }
}
