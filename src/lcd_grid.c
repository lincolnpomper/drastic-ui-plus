#include "lcd_grid.h"
#include "lcd_mask.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

struct lcd_grid_mask {
    SDL_Texture *texture;
    int width, height;
};

enum lcd_filter {
    LCD_OFF,
    LCD_GRID,
    LCD_1X,
    LCD_3X,
};

static struct {
    enum lcd_filter filter;
    struct lcd_mask_settings settings;
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

void lcd_grid_init(int native_width, int native_height) {
    const char *shader = getenv("DRASTIC_UI_PLUS_SHADER");
    lcd_grid.filter = LCD_OFF;
    if (shader) {
        if (!strcmp(shader, "lcd3x") || !strcmp(shader, "lcd-grid"))
            lcd_grid.filter = LCD_GRID;
        else if (!strcmp(shader, "retroarch-lcd1x"))
            lcd_grid.filter = LCD_1X;
        else if (!strcmp(shader, "retroarch-lcd3x"))
            lcd_grid.filter = LCD_3X;
    }
    /* Keep Original as the default for each filter. */
    const char *intensity = getenv("DRASTIC_UI_PLUS_LCD_INTENSITY");
    unsigned percent = 62;
    double strength = 1.0;
    if (intensity && !strcmp(intensity, "soft")) {
        percent = 85;
        strength = 0.5;
    } else if (intensity && !strcmp(intensity, "medium")) {
        percent = 75;
        strength = 0.75;
    } else if (intensity && !strcmp(intensity, "strong")) {
        percent = 50;
        strength = 2.0;
    }
    lcd_grid.settings = (struct lcd_mask_settings){
        .native_width = native_width,
        .native_height = native_height,
        .line_brightness = (255 * percent + 50) / 100,
        .crossing_brightness = (255 * percent * percent + 5000) / 10000,
        .column_depth = strength / (read_lcd_parameter("DRASTIC_UI_PLUS_BRIGHTEN_LCD", 4, 12) + 1),
        .row_depth = strength / (read_lcd_parameter("DRASTIC_UI_PLUS_BRIGHTEN_SCANLINES", 16, 32) + 1),
    };
}

bool lcd_grid_is_enabled(void) {
    return lcd_grid.filter != LCD_OFF;
}

void lcd_grid_clear_cache(void) {
    for (int screen = 0; screen < 2; screen++) {
        struct lcd_grid_mask *mask = &lcd_grid.masks[screen];
        if (mask->texture) SDL_DestroyTexture(mask->texture);
        *mask = (struct lcd_grid_mask){0};
    }
}

/* Cache the native-pixel pattern at the destination size. Each frame
   multiplies the game image by the mask with a single SDL blit. */
void draw_lcd_grid(SDL_Renderer *renderer, const SDL_Rect *rect, int screen,
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

        const struct lcd_mask_settings *settings = &lcd_grid.settings;
        bool filled = true;
        switch (lcd_grid.filter) {
        case LCD_GRID: fill_lcd_grid_mask(pixels, width, height, settings); break;
        case LCD_1X: filled = fill_lcd1x_mask(pixels, width, height, settings); break;
        case LCD_3X: filled = fill_lcd3x_mask(pixels, width, height, settings); break;
        case LCD_OFF: break;
        }
        if (!filled) {
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
