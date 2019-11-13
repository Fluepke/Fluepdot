#pragma once

#include <mcufont.h>

/***************************************
 * Parsing of the command line options *
 ***************************************/

typedef struct {
    const char *fontname;
    const char *filename;
    const char *text;
    bool justify;
    enum mf_align_t alignment;
    int width;
    int margin;
    int anchor;
    int scale;
} options_t;

/********************************************
 * Callbacks to specify rendering behaviour *
 ********************************************/

typedef struct {
    options_t *options;
    uint8_t *buffer;
    uint16_t width;
    uint16_t height;
    uint16_t y;
    const struct mf_font_s *font;
} state_t;

/* Callback to write to a memory buffer. */
void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha,
                           void *state);

/* Callback to just count x pixels
 * Used to decide buffer size */
void pixel_callback_width(int16_t x, int16_t y, uint8_t count, uint8_t alpha,
                           void *state);
/* Callback to render characters. */
uint8_t character_callback(int16_t x, int16_t y, mf_char character,
                                  void *state);

/* Callback to render lines. */
bool line_callback(const char *line, uint16_t count, void *state);

/* Callback to just count the lines.
 * Used to decide the image height */
bool count_lines(const char *line, uint16_t count, void *state);

