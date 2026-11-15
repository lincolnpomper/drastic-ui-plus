#ifndef DRASTIC_UI_PLUS_LCD_GRID_H
#define DRASTIC_UI_PLUS_LCD_GRID_H

#include <SDL2/SDL.h>
#include <stdbool.h>

void lcd_grid_init(int native_width, int native_height);
bool lcd_grid_is_enabled(void);
void lcd_grid_clear_cache(void);
void draw_lcd_grid(SDL_Renderer *renderer, const SDL_Rect *rect, int screen,
                   int (*render_copy)(SDL_Renderer *, SDL_Texture *,
                                      const SDL_Rect *, const SDL_Rect *));

#endif
