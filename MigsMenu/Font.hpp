/*
 * Author: Dylan Turner
 * Description:
 * - The GPU shows sprite data, so make a font for the sprite
 * - Sprites are 8x8 in rgab5515 format:
 *   + 2 byte pixels, little endian!
 *   + 15-11 => r
 *   + 10-6 => g
 *   + 5 => a
 *   + 4-0 => b
 */

#pragma once

#define FONT_SPACE          0
#define FONT_PERIOD         1
#define FONT_NUM_START      2
#define FONT_CAP_START      12
#define FONT_UNDER_START    38
#define FONT_LOW_START      39

namespace font {
    extern const char g_fontSprs[65][128];
}
