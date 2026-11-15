#include "lcd_mask.h"

#include <math.h>
#include <stdlib.h>

/* Based on Gigaherz's public-domain lcd3x.slang from libretro/slang-shaders. */
bool fill_lcd3x_mask(uint32_t *pixels, int width, int height,
                     const struct lcd_mask_settings *settings) {
    const double pi = 3.141592653589793;
    double (*columns)[3] = malloc((size_t)width * sizeof(*columns));
    if (!columns) return false;

    for (int x = 0; x < width; x++) {
        double angle = 2 * pi * ((x + 0.5) * settings->native_width / width);
        for (int channel = 0; channel < 3; channel++) {
            double phase = pi * (0.5 - 2.0 * channel / 3);
            columns[x][channel] = fmax(0, 1 - settings->column_depth * (1 - sin(angle + phase)));
        }
    }

    for (int y = 0; y < height; y++) {
        double angle = 2 * pi * ((y + 0.5) * settings->native_height / height);
        double row = fmax(0, 1 - settings->row_depth * (1 - sin(angle)));
        for (int x = 0; x < width; x++) {
            uint32_t red = (uint32_t)(255 * row * columns[x][0] + 0.5);
            uint32_t green = (uint32_t)(255 * row * columns[x][1] + 0.5);
            uint32_t blue = (uint32_t)(255 * row * columns[x][2] + 0.5);
            pixels[(size_t)y * width + x] = red << 24 | green << 16 | blue << 8 | 0xffu;
        }
    }
    free(columns);
    return true;
}
