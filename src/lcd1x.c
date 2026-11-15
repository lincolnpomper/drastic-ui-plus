#include "lcd_mask.h"

#include <math.h>
#include <stdlib.h>

/* Use the public-domain LCD3x scanline wave and one centered luminance
   aperture per pixel, shared equally by red, green, and blue. */
bool fill_lcd1x_mask(uint32_t *pixels, int width, int height,
                     const struct lcd_mask_settings *settings) {
    const double tau = 6.283185307179586;
    double *aperture = malloc((size_t)width * sizeof(*aperture));
    if (!aperture) return false;

    for (int x = 0; x < width; x++) {
        double angle = tau * ((x + 0.5) * settings->native_width / width);
        aperture[x] = fmax(0, 1 - settings->column_depth * (1 + cos(angle)));
    }

    for (int y = 0; y < height; y++) {
        double angle = tau * ((y + 0.5) * settings->native_height / height);
        double row = fmax(0, 1 - settings->row_depth * (1 - sin(angle)));
        for (int x = 0; x < width; x++) {
            uint32_t gray = (uint32_t)(255 * row * aperture[x] + 0.5);
            pixels[(size_t)y * width + x] = gray * 0x01010100u | 0xffu;
        }
    }
    free(aperture);
    return true;
}
