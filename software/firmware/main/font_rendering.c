#include "font_rendering.h"

void font_rendering_pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state)
{
    font_rendering_state_t* font_rendering_state = (font_rendering_state_t*)state;

    while (count--) {
        flipdot_framebuffer_set_pixel(font_rendering_state->framebuffer, x++, 15 - y, alpha > 128);
    }
}

uint8_t character_callback(int16_t x0, int16_t y0, mf_char character, void *state) {
    font_rendering_state_t* font_rendering_state = (font_rendering_state_t*)state;

    return mf_render_character(font_rendering_state->font, x0, y0, character, font_rendering_pixel_callback, state);
}
