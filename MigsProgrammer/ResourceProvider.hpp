/*
 * Author: Dylan Turner
 * Description:
 * - When not reprogramming the CPU, we want the programmer to provide
 *   resources from the SD card
 * - As long as we don't force a reset, we can continue to use Serial
 */

#pragma once

namespace rsrc {
    class ResourceProvider {
        public:
            ResourceProvider(void);
            void provide(void); // Loops until 0x55 is sent over serial
    };
}
