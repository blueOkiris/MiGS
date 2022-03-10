/*
 * Author: Dylan Turner
 * Description:
 * - Entry point for GPU system
 * - Note the sprites are in the following RGAB5515 format:
 *   + 2 byte pixels, little endian!
 *   + 15-11 => r
 *   + 10-6 => g
 *   + 5 => a
 *   + 4-0 => b
 */

// This is all C code, so use extern "C" so func names are preserved
extern "C" {
    #include <stdio.h>
    #include <pico/stdlib.h>
    #include <pico/multicore.h>
    #include <hardware/vreg.h>
    #include <hardware/irq.h>
    #include <hardware/sync.h>
    #include <hardware/i2c.h>
    #include <dvi.h>
    #include <dvi_timing.h>
    #include <sprite.h>
    #include <common_dvi_pin_configs.h>
}
#include <vector>

// DVDD 1.2V
#define VREG_VSEL       VREG_VOLTAGE_1_20
#define DVI_TIMING      dvi_timing_960x540p_60hz

// I only program in 'MURICAN
#define q_color_free    q_colour_free
#define q_color_valid   q_colour_valid

void initDvi(void);
void drawTextScanline(int y);
void drawScanline(const uint16_t *scanLine);

void initI2c(void);
char detectCpu(void);

const int g_cpuI2cAddr = 0x7C;
const int g_frameWidth = 480;
const int g_frameHeight = 270;
const int g_scanBuffCount = 4;

struct SprBuff {
    uint8_t data[8 * 8 * 2];
};

dvi_inst g_dvi;
uint16_t g_staticScanBuff[g_scanBuffCount][g_frameWidth];
std::vector<sprite_t> g_sprs;
std::vector<SprBuff> g_sprData;
uint16_t g_bg = 0x0000;

// Do color buff/init in Core1
void core1_main(void) {
    // Try to set it up
    dvi_register_irqs_this_core(&g_dvi, DMA_IRQ_0);
    while(queue_is_empty(&g_dvi.q_color_valid)) {
        __wfe(); // "Wait For Event"
    }

    dvi_start(&g_dvi);
    dvi_scanbuf_main_16bpp(&g_dvi); // This is an infinite loop
}

int main() {
    // Speed up the clock
    vreg_set_voltage(VREG_VSEL);
    sleep_ms(10);
    set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

    stdio_init_all();
    setup_default_uart();

    initI2c();

    initDvi();
    multicore_launch_core1(core1_main);

    // Allocate scanline buffer
    for (int i = 0; i < g_scanBuffCount; ++i) {
        void *buffPtr = &g_staticScanBuff[i];
        queue_add_blocking((queue_t *) &g_dvi.q_color_free, &buffPtr);
    }

    SprBuff sprBuff;
    for(int i = 0; i < 8 * 8 * 2; i += 2) {
        sprBuff.data[i] = 0b11100000;
        sprBuff.data[i + 1] = 0b00000111;
    }
    g_sprData.push_back(sprBuff);
    for(int y = 0; y < 3; y++) {
        for(int x = 0; x < g_frameWidth / 8; x += 3) {
            g_sprs.push_back(sprite_t {
                x * 8, y * 8,
                g_sprData[g_sprData.size() - 1].data,
                3, false,
                false, false
            });
        }
    }

    uint8_t drawCmd[128];
    uint8_t delayCtr = 0; // Only read updates every fractional time
    while(true) {
        for(int y = 0; y < g_frameHeight; y++) {
            uint16_t *pixBuff = nullptr;
            queue_remove_blocking(&g_dvi.q_color_free, &pixBuff);
            sprite_fill16(pixBuff, g_bg, g_frameWidth);
            for(size_t i = 0; i < g_sprs.size(); i++) {
                sprite_sprite16(pixBuff, &g_sprs[i], y, g_frameWidth);
            }
            queue_add_blocking(&g_dvi.q_color_valid, &pixBuff);
        }

        if(delayCtr++ % 40 == 0) {
            if(i2c_read_blocking(i2c1, g_cpuI2cAddr, drawCmd, 1, false) > 0) {
                switch(drawCmd[0]) {
                    // Do nothing
                    case 0x55:
                        break;

                    // Draw text to screen
                    case 'T': {
                        
                    } break;

                    case 'B':
                        i2c_read_blocking(
                            i2c1, g_cpuI2cAddr, drawCmd, 2, false
                        );
                        g_bg = (((uint16_t) drawCmd[0]) << 8) + drawCmd[1];
                        break;
                }
            }
        }
    }

    return 0;
}

// DVI-specific functions
void initDvi(void) {
    // Set up DVI
    g_dvi.timing = &DVI_TIMING;
    g_dvi.ser_cfg = DVI_DEFAULT_SERIAL_CONFIG;
    dvi_init(
        &g_dvi,
        next_striped_spin_lock_num(),
        next_striped_spin_lock_num()
    );
}

void initI2c(void) {
    i2c_init(i2c1, 100 * 1000);
    gpio_set_function(2, GPIO_FUNC_I2C);
    gpio_set_function(3, GPIO_FUNC_I2C);
    gpio_pull_up(2);
    gpio_pull_up(3);
}
