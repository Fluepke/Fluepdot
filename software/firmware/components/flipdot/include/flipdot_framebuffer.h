/**
  * @file
  * @brief Framebuffer structs and manipulation methods
  */

#pragma once

#include <stdbool.h>
#include "esp_log.h"
#include "esp_err.h"

#define FLIPDOT_PIXEL_SET 'X'
#define FLIPDOT_PIXEL_CLEAR ' '

/**
  * A framebuffer with bottom left corner as x=0, y=0.
  */
typedef struct {
    /**
      * 16 pixels high columns
      */
    uint16_t* columns;
    
    /**
      * Framebuffer width
      */
    uint8_t width;
} framebuffer_t;


/**
  * @brief Initialize a framebuffer
  * @param framebuffer The framebuffer_t* to initialize
  * @param width Framebuffer width. Must be > 0
  * @return
  *   - `ESP_ERR_INVALID_ARG`: framebuffer was NULL or width was 0
  *   - `ESP_ERR_NO_MEM`: failed to allocate memory
  */
esp_err_t flipdot_framebuffer_init(framebuffer_t* framebuffer, uint8_t width);

/**
  * @brief Clear a framebuffer (set all pixels dark)
  * @param framebuffer The framebuffer to clear
  */
void flipdot_framebuffer_clear(framebuffer_t* framebuffer);

/**
  * @brief Get a single pixel value
  * @param framebuffer The framebuffer to retrieve pixel value from
  * @param x X location, @see framebuffer_t for an explanation of the coordinate system
  * @param y Y location, @see framebuffer_t for an explanation of the coordinate system
  * @return
  *   - true Pixel is set (bright)
  *   - false Pixel is not set (dark) or out of bounds read or framebuffer was NULL
  */
bool flipdot_framebuffer_get_pixel(framebuffer_t* framebuffer, uint8_t x, uint8_t y);

/**
  * @brief Set a single pixel value
  * @param framebuffer The framebuffer to manipulate
  * @param x X location, @see framebuffer_t for an explanation of the coordinate system
  * @param y Y location, @see framebuffer_t for an explanation of the coordinate system
  * @param value
  *   - true Pixel is set (bright)
  *   - false Pixel is not set (dark) or out of bounds read
  * @return
  *   - ESP_OK success
  *   - ESP_ERR_INVALID_ARG out of bounds or framebuffer was NULL
  */
esp_err_t flipdot_framebuffer_set_pixel(framebuffer_t* framebuffer, uint8_t x, uint8_t y, bool value);

/**
  * @brief ASCII encode a horizontal framebuffer line
  * @param framebuffer The framebuffer to encode
  * @param dst Destination char buffer
  * @param dst_len Destination buffer size, must be framebuffer->width + 1 to accomodate the trailing 0 character
  * @return
  *   - NULL failure
  *   - char* Pointer to the encoded line
  * @note @see flipdot_framebuffer_encode_pixel is used for ASCII encoding single pixels
  */
char* flipdot_framebuffer_encode_line(framebuffer_t* framebuffer, char* dst, size_t dst_len, uint8_t y);

/**
  * @brief ASCII decode a horizontal framebuffer line
  * @param framebuffer The framebuffer to decode into
  * @param src Source char buffer
  * @param src_len Source buffer size, must be framebuffer->width + 1 to accomodate the trailing 0 character
  * @return
  *   - ESP_OK success
  *   - ESP_INVALID_ARG framebuffer was null, or src_len != framebuffer->width + 1
  * @note @see flipdot_framebuffer_decode_pixel is used for ASCII decoding single pixels
  */
esp_err_t flipdot_framebuffer_decode_line(framebuffer_t* framebuffer, char* src, size_t src_len, uint8_t y);

/**
  * @brief ASCII encode a single pixel
  * @param value The pixel value to encode
  * @return
  *   - @see FLIPDOT_PIXEL_SET pixel is set
  *   - @see FLIPDOT_PIXEL_CLEAR pixel is not set
  */
char flipdot_framebuffer_encode_pixel(bool value);

/**
  * @brief ASCII decode a single pixel
  * @param src Character to decode into a pixel value
  * @return
  *   - true pixel is set (src == FLIPDOT_PIXEL_SET)
  *   - false pixel is not set (all other cases)
  */ 
bool flipdot_framebuffer_decode_pixel(char src);

/**
  * @brief Printf the given framebuffer to STDOUT
  * @param framebuffer The framebuffer to print
  * @return
  *   - ESP_OK success
  *   - ESP_ERR_INVALID_ARG framebuffer was NULL
  *   - ESP_ERR_NO_MEM could not allocate enough memory
  */
esp_err_t flipdot_framebuffer_printf(framebuffer_t* framebuffer);

/**
  * @brief Copies a framebuffer_t* from src to dest
  * @param dest Pointer to the destintation framebuffer_t, must not be null
  * @param src Pointer to the source framebuffer_t
  * @note If src->columns != NULL src->columns will be free'd
  * @return
  *   - ESP_ERR_INVALID_ARG dest was NULL and / or src was NULL and / or src->width == 0
  *   - ESP_ERR_NO_MEM could not allocate required memory
  *   - ESP_OK success
  */
esp_err_t flipdot_framebuffer_copy(framebuffer_t* dest, framebuffer_t* src);

/**
  * @brief Compares two framebuffers
  * @param a Framebuffer 1
  * @param b Framebuffer 2
  * @param diff Pointer to an integer to store the number of changed pixels
  * @return
  *   - ESP_INVALID_ARG a was NULL and / or b was NULL and / or diff was NULL and / or  a->width != b->width
  *   - ESP_OK success
  */
esp_err_t flipdot_framebuffer_compare(framebuffer_t* a, framebuffer_t* b, unsigned int* diff);

/**
  * @brief Compares two framebuffers partially
  * @param a Framebuffer 1
  * @param b Framebuffer 2
  * @param start_x X offset from where to start comparing
  * @param width Width of the view port to compare
  * @param diff Pointer to an integer to store the number of changed pixels
  * @return
  *   - ESP_INVALID_ARG a was NULL and / or b was NULL and / or diff was NULL and / or a->width != b->width and / or a->width < start_x + width
  *   - ESP_OK success
  */
esp_err_t flipdot_framebuffer_compare_partial(framebuffer_t* a, framebuffer_t* b, uint8_t start_x, uint8_t width, unsigned int* diff);

/**
  * @brief Free a framebuffer_t*
  * Framebuffer is freed and the pointer set to NULL to avoid use after free bugs
  * @param framebuffer The framebuffer to free
  */
void flipdot_framebuffer_free(framebuffer_t* framebuffer);
