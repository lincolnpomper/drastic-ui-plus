#ifndef DRASTIC_UI_PLUS_LCD_MASK_H
#define DRASTIC_UI_PLUS_LCD_MASK_H

#include <stdbool.h>
#include <stdint.h>

struct lcd_mask_settings {
    int native_width, native_height;
    unsigned line_brightness, crossing_brightness;
    double column_depth, row_depth;
};

void fill_lcd_grid_mask(uint32_t *pixels, int width, int height,
                        const struct lcd_mask_settings *settings);
bool fill_lcd1x_mask(uint32_t *pixels, int width, int height,
                     const struct lcd_mask_settings *settings);
bool fill_lcd3x_mask(uint32_t *pixels, int width, int height,
                     const struct lcd_mask_settings *settings);

#endif
