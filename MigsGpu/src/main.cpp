/*
 * Author: Dylan Turner
 * Description: Entry point for GPU system
 */

#include <stdio.h>
#include <pico/stdlib.h>

int main() {
    stdio_init_all();
    while(true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
    return 0;
}
