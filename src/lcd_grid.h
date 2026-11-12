#ifndef DRASTIC_UI_PLUS_LCD_GRID_H
#define DRASTIC_UI_PLUS_LCD_GRID_H

#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Private implementation included by drastic-ui-plus.c so the hook still builds
   from a single translation unit. */
struct lcd_grid_mask {
    SDL_Texture *texture;
    int width, height;
};

enum lcd_filter {
    LCD_OFF,
    LCD_GRID,
    LCD_RETROARCH_1X,
    LCD_RETROARCH_3X,
};

static struct {
    enum lcd_filter filter;
    int native_width, native_height;
    unsigned line_brightness, crossing_brightness;
    double retroarch_strength;
    double brighten_scanlines, brighten_lcd;
    struct lcd_grid_mask masks[2];
} lcd_grid;

static double read_lcd_parameter(const char *name, double fallback, double max) {
    const char *value = getenv(name);
    if (!value || !*value) return fallback;
    char *end;
    double number = strtod(value, &end);
    return *end == '\0' && isfinite(number) && number >= 1 && number <= max
        ? number : fallback;
}

static void lcd_grid_init(int native_width, int native_height) {
    const char *shader = getenv("DRASTIC_UI_PLUS_SHADER");
    lcd_grid.filter = LCD_OFF;
    if (shader) {
        if (!strcmp(shader, "lcd3x") || !strcmp(shader, "lcd-grid"))
            lcd_grid.filter = LCD_GRID;
        else if (!strcmp(shader, "retroarch-lcd1x"))
            lcd_grid.filter = LCD_RETROARCH_1X;
        else if (!strcmp(shader, "retroarch-lcd3x"))
            lcd_grid.filter = LCD_RETROARCH_3X;
    }
    /* Keep Original as the default for each filter. */
    const char *intensity = getenv("DRASTIC_UI_PLUS_LCD_INTENSITY");
    unsigned percent = 62;
    lcd_grid.retroarch_strength = 1.0;
    if (intensity && !strcmp(intensity, "soft")) {
        percent = 85;
        lcd_grid.retroarch_strength = 0.5;
    } else if (intensity && !strcmp(intensity, "medium")) {
        percent = 75;
        lcd_grid.retroarch_strength = 0.75;
    } else if (intensity && !strcmp(intensity, "strong")) {
        percent = 50;
        lcd_grid.retroarch_strength = 2.0;
    }
    lcd_grid.line_brightness = (255 * percent + 50) / 100;
    lcd_grid.crossing_brightness = (255 * percent * percent + 5000) / 10000;
    lcd_grid.brighten_scanlines = read_lcd_parameter("DRASTIC_UI_PLUS_BRIGHTEN_SCANLINES", 16, 32);
    lcd_grid.brighten_lcd = read_lcd_parameter("DRASTIC_UI_PLUS_BRIGHTEN_LCD", 4, 12);
    lcd_grid.native_width = native_width;
    lcd_grid.native_height = native_height;
}

static bool lcd_grid_is_enabled(void) {
    return lcd_grid.filter != LCD_OFF;
}

static void lcd_grid_clear_cache(void) {
    for (int screen = 0; screen < 2; screen++) {
        struct lcd_grid_mask *mask = &lcd_grid.masks[screen];
        if (mask->texture) SDL_DestroyTexture(mask->texture);
        *mask = (struct lcd_grid_mask){0};
    }
}

/* Masks from libretro/slang-shaders handheld/shaders/lcd1x.slang
   (jdgleaver) and lcd3x.slang (Gigaherz). Evaluate at output pixel centers.
   The effect is separable, so sine factors can be computed per axis. */
static bool fill_retroarch_lcd_mask(uint32_t *pixels, int width, int height) {
    const double pi = 3.141592653589793;
    bool rgb = lcd_grid.filter == LCD_RETROARCH_3X;
    // Scale the darkening from white while keeping the reference phases.
    double column_depth = lcd_grid.retroarch_strength / (lcd_grid.brighten_lcd + 1);
    double row_depth = lcd_grid.retroarch_strength / (lcd_grid.brighten_scanlines + 1);
    double (*columns)[3] = malloc((size_t)width * sizeof(*columns));
    if (!columns) return false;

    // Strong with low Brighten values can bring the darkest samples to black.
    for (int x = 0; x < width; x++) {
        double angle = 2 * pi * ((x + 0.5) * lcd_grid.native_width / width);
        if (rgb) {
            for (int channel = 0; channel < 3; channel++) {
                double phase = pi * (0.5 - 2.0 * channel / 3);
                columns[x][channel] = fmax(0, 1 - column_depth * (1 - sin(angle + phase)));
            }
        } else {
            double factor = fmax(0, 1 - column_depth * (1 - sin(angle - pi / 2)));
            columns[x][0] = columns[x][1] = columns[x][2] = factor;
        }
    }

    for (int y = 0; y < height; y++) {
        double angle = 2 * pi * ((y + 0.5) * lcd_grid.native_height / height);
        double factor = fmax(0, 1 - row_depth * (1 - sin(angle - (rgb ? 0 : pi / 2))));
        for (int x = 0; x < width; x++) {
            uint32_t red = (uint32_t)(255 * factor * columns[x][0] + 0.5);
            uint32_t green = (uint32_t)(255 * factor * columns[x][1] + 0.5);
            uint32_t blue = (uint32_t)(255 * factor * columns[x][2] + 0.5);
            pixels[(size_t)y * width + x] = red << 24 | green << 16 | blue << 8 | 0xffu;
        }
    }
    free(columns);
    return true;
}

static void fill_lcd_grid_mask(uint32_t *pixels, int width, int height) {
    int native_width = lcd_grid.native_width;
    int native_height = lcd_grid.native_height;
    for (int y = 0; y < height; y++) {
        bool row = (2 * y + 1) * native_height % (2 * height) <
                   SDL_min(height, 2 * native_height);
        for (int x = 0; x < width; x++) {
            bool column = (2 * x + 1) * native_width % (2 * width) <
                          SDL_min(width, 2 * native_width);
            uint32_t gray = row && column ? lcd_grid.crossing_brightness :
                            (row || column ? lcd_grid.line_brightness : 255);
            pixels[(size_t)y * width + x] = gray * 0x01010100u | 0xffu;
        }
    }
}

/* Cache the native-pixel pattern at the destination size. Each frame
   multiplies the game image by the mask with a single SDL blit. */
static void draw_lcd_grid(SDL_Renderer *renderer, const SDL_Rect *rect, int screen,
                          int (*render_copy)(SDL_Renderer *, SDL_Texture *,
                                             const SDL_Rect *, const SDL_Rect *)) {
    if (!lcd_grid_is_enabled() || rect->w <= 0 || rect->h <= 0) return;

    int width = rect->w, height = rect->h;
    struct lcd_grid_mask *mask = &lcd_grid.masks[screen];
    if (mask->width != width || mask->height != height) {
        if (mask->texture) SDL_DestroyTexture(mask->texture);
        *mask = (struct lcd_grid_mask){.width = width, .height = height};

        uint32_t *pixels = malloc((size_t)width * height * sizeof(*pixels));
        if (!pixels) return;

        if (lcd_grid.filter == LCD_GRID) {
            fill_lcd_grid_mask(pixels, width, height);
        } else if (!fill_retroarch_lcd_mask(pixels, width, height)) {
            free(pixels);
            return;
        }

        SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                                 SDL_TEXTUREACCESS_STATIC, width, height);
        if (texture && SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(*pixels)) == 0 &&
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_MOD) == 0) {
            mask->texture = texture;
        } else if (texture) {
            SDL_DestroyTexture(texture);
        }
        free(pixels);
    }
    if (mask->texture) render_copy(renderer, mask->texture, NULL, rect);
}

#endif
