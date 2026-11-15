#include "lcd_mask.h"

#include <stddef.h>

void fill_lcd_grid_mask(uint32_t *pixels, int width, int height,
                        const struct lcd_mask_settings *settings) {
    int native_width = settings->native_width;
    int native_height = settings->native_height;
    int row_limit = height < 2 * native_height ? height : 2 * native_height;
    int column_limit = width < 2 * native_width ? width : 2 * native_width;
    for (int y = 0; y < height; y++) {
        bool row = (2 * y + 1) * native_height % (2 * height) <
                   row_limit;
        for (int x = 0; x < width; x++) {
            bool column = (2 * x + 1) * native_width % (2 * width) <
                          column_limit;
            uint32_t gray = row && column ? settings->crossing_brightness :
                            (row || column ? settings->line_brightness : 255);
            pixels[(size_t)y * width + x] = gray * 0x01010100u | 0xffu;
        }
    }
}
