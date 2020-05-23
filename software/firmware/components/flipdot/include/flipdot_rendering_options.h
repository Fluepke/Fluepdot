/**
  * @file
  * @brief Rendering options
  */

#pragma once

#include "flipdot_gpio.h"
#include "flipdot_rendering_options.h"

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_err.h"

// How long to wait before rendering a column (in 10ms counts)
#define FLIPDOT_RENDERING_OPTIONS_PRE_DELAY_DEFAULT 0

// How long to power the column clear low side driver by default (in 10ms counts)
#define FLIPDOT_RENDERING_OPTIONS_SET_DELAY_DEFAULT 160

// How long to power the row high side drivers by default (in 10ms counts)
#define FLIPDOT_RENDERING_OPTIONS_CLEAR_DELAY_DEFAULT 160

// Flipdot rendering mode
typedef enum {
    /**
      * Always render the full framebuffer without skipping anything
      */
    FULL = 0,

    /**
      * Redraw only those pixels that changed
      */
    DIFFERENTIAL = 1
} flipdot_rendering_mode_t;

/**
  * Flipdot timing options
  */
typedef struct {
    /**
      * How long to wait before rendering a column (in 10ms counts)
      */
    uint16_t pre_delay;

    /**
      * How long to power the row high side drivers (in 10ms counts)
      */
    uint16_t clear_delay;
    
    /**
      * How long to power the column clear low side driver (in 10ms counts)
      */
    uint16_t set_delay;
} flipdot_rendering_delay_options_t;

/**
  * Flipdot rendering options
  */
typedef struct {
    /**
      * Flipdot width
      */
    uint8_t width;

    /**
      * Number of panels attached to flipdot
      */
    uint8_t panel_count;

    /**
      * Flipdot rendering mode
      */
    flipdot_rendering_mode_t mode;

    /**
      * Flipdot timing options
      */
    flipdot_rendering_delay_options_t* delay_options;

    /**
      * Order in which panels are rendered
      */
    uint8_t* panel_order;
} flipdot_rendering_options_t;

/**
  * Allocates internal data structures and initializes them with reasonable defaults
  * @param options The flipdot_rendering_options_t to initialize, must not be NULL.
  * @param panel_count Number of panels available, must be larger than 0
  * @param width Number of columns available, must be larger than 0
  * @return
  *  - `ESP_ERR_INVALID_ARG`: rendering_options was NULL, panel_count was 0 or  width was 0
  *  - `ESP_ERR_NO_MEM`: could not allocate memory for internal data structures
  *  - `ESP_OK`: success
  */
esp_err_t flipdot_rendering_options_initialize(flipdot_rendering_options_t* rendering_options, uint8_t panel_count, uint8_t width);

/**
  * Copies rendering options from src to dest
  * @param dest Allocate using `calloc(1, sizeof(flipdot_rendering_options_t))`
  * @param src Source
  */
esp_err_t flipdot_rendering_options_copy(flipdot_rendering_options_t* dest, flipdot_rendering_options_t* src);

/**
  * Free a flipdot_rendering_options_t*
  * @param rendering_options flipdot_rendering_options_t* to free.
  */
void flipdot_rendering_options_free(flipdot_rendering_options_t* rendering_options);
