#pragma once

#include "flipdot_framebuffer.h"
#include <mcufont.h>

typedef struct {
    framebuffer_t* framebuffer;
    const struct mf_font_s* font;
} font_rendering_state_t;

/* Callback function that writes pixels to screen / buffer / whatever.
 *
 * x:     X coordinate of the first pixel to write.
 * y:     Y coordinate of the first pixel to write.
 * count: Number of pixels to fill (horizontally).
 * alpha: The "opaqueness" of the pixels, 0 for background, 255 for text.
 * state: Free variable that was passed to render_character().
 */
void font_rendering_pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state);

/* Callback for rendering a single character.
 * x0:          Left edge of the target position of character.
 * y0:          Upper edge of the target position of character.
 * character:   Character to render.
 * state:       Free state variable for use by the callback.
 * Returns the width of the character.
 */
uint8_t character_callback(int16_t x0, int16_t y0, mf_char character, void *state);
