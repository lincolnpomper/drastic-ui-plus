/* Run with SDL's software renderer; no emulator or display is needed. */
#include <assert.h>
#include <stdio.h>
#include "../src/lcd_grid.h"

static void check_mask(SDL_Renderer *renderer, const char *preset,
                       unsigned line, unsigned crossing) {
    if (preset) setenv("DRASTIC_UI_PLUS_LCD_INTENSITY", preset, 1);
    else unsetenv("DRASTIC_UI_PLUS_LCD_INTENSITY");
    lcd_grid_clear_cache();
    lcd_grid_init(256, 192);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_Rect rect = {0, 0, 1024, 768};
    draw_lcd_grid(renderer, &rect, 0, SDL_RenderCopy);
    uint32_t *pixels = malloc(1024 * 768 * sizeof(*pixels));
    assert(pixels);
    assert(SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA8888,
                                pixels, 1024 * sizeof(*pixels)) == 0);
    for (int y = 0; y < 768; ++y) {
        for (int x = 0; x < 1024; ++x) {
            unsigned expected = !lcd_grid_is_enabled() ? 255 :
                (x % 4 == 0 && y % 4 == 0) ? crossing :
                (x % 4 == 0 || y % 4 == 0) ? line : 255;
            uint32_t pixel = pixels[y * 1024 + x];
            assert((pixel >> 24) == expected);
            assert(((pixel >> 16) & 255) == expected);
            assert(((pixel >> 8) & 255) == expected);
        }
    }
    free(pixels);
}

static void check_retroarch_mask(SDL_Renderer *renderer, const char *filter,
                                 const char *preset, const uint32_t expected[4]) {
    setenv("DRASTIC_UI_PLUS_SHADER", filter, 1);
    setenv("DRASTIC_UI_PLUS_LCD_INTENSITY", preset, 1);
    lcd_grid_clear_cache();
    lcd_grid_init(256, 192);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    SDL_Rect screen = {0, 0, 1024, 768};
    SDL_Rect first_row = {0, 0, 4, 1};
    uint32_t pixels[4];
    draw_lcd_grid(renderer, &screen, 0, SDL_RenderCopy);
    assert(SDL_RenderReadPixels(renderer, &first_row, SDL_PIXELFORMAT_RGBA8888,
                                pixels, sizeof(pixels)) == 0);
    // Reference shader outputs for white at the first four 4x sample centers.
    for (int x = 0; x < 4; x++) assert(pixels[x] == expected[x]);
}

int main(void) {
    unsetenv("DRASTIC_UI_PLUS_BRIGHTEN_SCANLINES");
    unsetenv("DRASTIC_UI_PLUS_BRIGHTEN_LCD");
    assert(SDL_Init(0) == 0);
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(0, 1024, 768, 32,
                                                        SDL_PIXELFORMAT_RGBA8888);
    assert(surface);
    SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(surface);
    assert(renderer);
    const char *shaders[] = {"lcd3x", "lcd-grid", "none", "auto", "unknown", NULL};
    for (unsigned i = 0; i < sizeof(shaders) / sizeof(*shaders); ++i) {
        if (shaders[i]) setenv("DRASTIC_UI_PLUS_SHADER", shaders[i], 1);
        else unsetenv("DRASTIC_UI_PLUS_SHADER");
        check_mask(renderer, "soft", 217, 184);
        check_mask(renderer, "medium", 191, 143);
        check_mask(renderer, "original", 158, 98);
        check_mask(renderer, "strong", 128, 64);
        check_mask(renderer, "invalid", 158, 98);
        check_mask(renderer, NULL, 158, 98);
    }
    check_retroarch_mask(renderer, "retroarch-lcd1x", "soft",
                        (const uint32_t[]){0xc9c9c9ff, 0xebebebff, 0xebebebff, 0xc9c9c9ff});
    check_retroarch_mask(renderer, "retroarch-lcd1x", "medium",
                        (const uint32_t[]){0xafafafff, 0xe1e1e1ff, 0xe1e1e1ff, 0xafafafff});
    check_retroarch_mask(renderer, "retroarch-lcd1x", "original",
                        (const uint32_t[]){0x979797ff, 0xd8d8d8ff, 0xd8d8d8ff, 0x979797ff});
    check_retroarch_mask(renderer, "retroarch-lcd1x", "strong",
                        (const uint32_t[]){0x414141ff, 0xb4b4b4ff, 0xb4b4b4ff, 0x414141ff});
    check_retroarch_mask(renderer, "retroarch-lcd3x", "soft",
                        (const uint32_t[]){0xf5eacbff, 0xd2fcddff, 0xd2ddfcff, 0xf5cbeaff});
    check_retroarch_mask(renderer, "retroarch-lcd3x", "medium",
                        (const uint32_t[]){0xf1e0b1ff, 0xbbfaccff, 0xbbccfaff, 0xf1b1e0ff});
    check_retroarch_mask(renderer, "retroarch-lcd3x", "original",
                        (const uint32_t[]){0xecd598ff, 0xa5f9bcff, 0xa5bcf9ff, 0xec98d5ff});
    check_retroarch_mask(renderer, "retroarch-lcd3x", "strong",
                        (const uint32_t[]){0xd9ad35ff, 0x4ef37aff, 0x4e7af3ff, 0xd935adff});
    setenv("DRASTIC_UI_PLUS_BRIGHTEN_SCANLINES", "1.0", 1);
    check_retroarch_mask(renderer, "retroarch-lcd1x", "original",
                        (const uint32_t[]){0x191919ff, 0x232323ff, 0x232323ff, 0x191919ff});
    setenv("DRASTIC_UI_PLUS_BRIGHTEN_SCANLINES", "nan", 1);
    setenv("DRASTIC_UI_PLUS_BRIGHTEN_LCD", "1.0", 1);
    check_retroarch_mask(renderer, "retroarch-lcd1x", "original",
                        (const uint32_t[]){0x222222ff, 0xc4c4c4ff, 0xc4c4c4ff, 0x222222ff});
    setenv("DRASTIC_UI_PLUS_BRIGHTEN_SCANLINES", "1.0", 1);
    check_retroarch_mask(renderer, "retroarch-lcd1x", "strong",
                        (const uint32_t[]){0xff, 0xff, 0xff, 0xff});
    lcd_grid_clear_cache();
    SDL_DestroyRenderer(renderer);
    SDL_FreeSurface(surface);
    SDL_Quit();
    puts("PASS: all output pixels, four intensities, defaults, disabled filter and RetroArch masks");
}
