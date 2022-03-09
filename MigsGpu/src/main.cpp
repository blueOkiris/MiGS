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

#define CPU_I2C_ADDR    0x7C

// DVDD 1.2V
#define FRAME_WIDTH     320
#define FRAME_HEIGHT    240
#define VREG_VSEL       VREG_VOLTAGE_1_20
#define DVI_TIMING      dvi_timing_640x480p_60hz

// I only program in 'MURICAN
#define q_color_free    q_colour_free
#define q_color_valid   q_colour_valid

dvi_inst g_dvi;

void init_dvi(void);
void start_dvi_signaling(void);
void draw_scanline(const uint16_t *scanLine);

void init_i2c(void);
char detect_cpu(void);

// Do color buff/init in Core1
void core1_main(void) {
    start_dvi_signaling();

    // TODO: Do other stuff
}

int main() {
    // Speed up the clock
    vreg_set_voltage(VREG_VSEL);
    sleep_ms(10);
    set_sys_clock_khz(DVI_TIMING.bit_clk_khz, true);

    setup_default_uart();

    init_i2c();
    while(!detect_cpu()) { // Wait for arduino sender
        sleep_ms(10);
    }

    init_dvi();
    multicore_launch_core1(core1_main);

    // Push data from image and then remove it to put the next one
    uint frame_ctr = 0;
    while(true) {
        for(uint y = 0; y < FRAME_HEIGHT; y++) { // Draw each line
            uint y_scroll = (y + frame_ctr) % FRAME_HEIGHT; // Relative to ctr

            // Get the right line and send to dvi
            const uint16_t *scanLine = &((const uint16_t *) testcard_320x240)[
                y_scroll * FRAME_WIDTH
            ];
            draw_scanline(scanLine);
        }
        frame_ctr++;
    }

    return 0;
}

// DVI-specific functions
void init_dvi(void) {
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
void start_dvi_signaling(void) {
    // Try to set it up
    dvi_register_irqs_this_core(&g_dvi, DMA_IRQ_0);
    while(queue_is_empty(&g_dvi.q_color_valid)) {
        __wfe(); // "Wait For Event"
    }

    dvi_start(&g_dvi);
    dvi_scanbuf_main_16bpp(&g_dvi);
}

// Push a scanline buffer
void draw_scanline(const uint16_t *scanLine) {
    queue_add_blocking_u32(&g_dvi.q_color_valid, &scanLine);
    while(queue_try_remove_u32(&g_dvi.q_color_free, &scanLine));
}

void init_i2c(void) {
    i2c_init(i2c1, 100 * 1000);
    gpio_set_function(2, GPIO_FUNC_I2C);
    gpio_set_function(3, GPIO_FUNC_I2C);
    gpio_pull_up(2);
    gpio_pull_up(3);
}

char detect_cpu(void) {
    uint8_t data = 0;
    return i2c_read_blocking(i2c1, CPU_I2C_ADDR, &data, 1, false) >= 0;
}
