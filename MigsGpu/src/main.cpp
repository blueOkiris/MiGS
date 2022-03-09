/*
 * Author: Dylan Turner
 * Description: Entry point for GPU system
 */

// This is all C code, so use extern "C" so func names are preserved
extern "C" {
    #include <stdio.h>
    #include <pico/stdlib.h>
    #include <pico/multicore.h>
    #include <hardware/vreg.h>
    #include <dvi.h>
    #include <dvi_timing.h>
    #include <hardware/irq.h>
    #include <hardware/sync.h>
    #include <hardware/i2c.h>
    #include <common_dvi_pin_configs.h>

    // TODO: This is temporary while testing. Will be removed
    #include "../../PicoDVI/software/assets/testcard_320x240_rgb565.h"
}

const int g_cpuI2cAddr = 0x7C;
const int g_frameWidth = 320;
const int g_frameHeight = 240;

// DVDD 1.2V
#define VREG_VSEL       VREG_VOLTAGE_1_20
#define DVI_TIMING      dvi_timing_640x480p_60hz

// I only program in 'MURICAN
#define q_color_free    q_colour_free
#define q_color_valid   q_colour_valid

dvi_inst g_dvi;

void initDvi(void);
void startDviSignaling(void);
void drawScanline(const uint16_t *scanLine);

void initI2c(void);
char detectCpu(void);

// Do color buff/init in Core1
void core1_main(void) {
    startDviSignaling();

    uint8_t drawCmd[128];
    while(true) {
        if(i2c_read_blocking(i2c1, g_cpuI2cAddr, drawCmd, 128, false) < 1) {
            continue;
        }

        switch(drawCmd[0]) {
            // Do nothing
            case 0x55:
                break;

            // Draw text to screen
            case 'T': {
                int x = ((uint8_t) drawCmd[1] << 8) + drawCmd[2];
                int y = drawCmd[3];
                char *text = (char *) (drawCmd + 4);

                //"drawText(x, y, text);"
            } break;
        }
    }
}

int main() {
    // Speed up the clock
    vreg_set_voltage(VREG_VSEL);
    sleep_ms(10);
    set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

    setup_default_uart();

    initI2c();

    initDvi();
    multicore_launch_core1(core1_main);

    // Push data from image and then remove it to put the next one
    uint frameCtr = 0;
    while(true) {
        for(uint y = 0; y < g_frameHeight; y++) { // Draw each line
            uint yScroll = (y + frameCtr) % g_frameHeight; // Relative to ctr

            // Get the right line and send to dvi
            const uint16_t *scanLine = &((const uint16_t *) testcard_320x240)[
                yScroll * g_frameWidth
            ];
            drawScanline(scanLine);
        }
        frameCtr++;
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

// Waits till it sees the first color buff and then starts up signal
void startDviSignaling(void) {
    // Try to set it up
    dvi_register_irqs_this_core(&g_dvi, DMA_IRQ_0);
    while(queue_is_empty(&g_dvi.q_color_valid)) {
        __wfe(); // "Wait For Event"
    }

    dvi_start(&g_dvi);
    dvi_scanbuf_main_16bpp(&g_dvi);
}

// Push a scanline buffer
void drawScanline(const uint16_t *scanLine) {
    queue_add_blocking_u32(&g_dvi.q_color_valid, &scanLine);
    while(queue_try_remove_u32(&g_dvi.q_color_free, &scanLine));
}

void initI2c(void) {
    i2c_init(i2c1, 100 * 1000);
    gpio_set_function(2, GPIO_FUNC_I2C);
    gpio_set_function(3, GPIO_FUNC_I2C);
    gpio_pull_up(2);
    gpio_pull_up(3);
}
