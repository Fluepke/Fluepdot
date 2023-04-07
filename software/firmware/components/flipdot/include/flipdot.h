/**
  * @file
  * @brief High level flipdot abstraction
  */
#pragma once

#include "freertos/FreeRTOS.h"
#include "driver/spi_master.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "flipdot_rendering_options.h"
#include "flipdot_gpio.h"
#include "flipdot_util.h"
#include "flipdot_framebuffer.h"

/**
  * Used to signal the @see flipdot.task that the framebuffer is dirty.
  */
#define FLIPDOT_FRAMEBUFFER_DIRTY_BIT BIT0
#define FLIPDOT_RENDERING_DONE_BIT    BIT1

/**
  * Limit imposed by hardware by default.
  */
#define FLIPDOT_MAX_SUPPORTED_PANELS (5)

/**
  * Location and size of a flipdot panel.
  * A flipdot consists of up to 5 panels.
  * Width of a panel is typically 20 or 25 pixels.
  * Height is hardcoded to be 16.
  */
typedef struct {
    /**
      * Panel width
      */
    uint8_t width;
    /**
      * Panel offset
      */
    uint8_t x;
} flipdot_panel_t;

/**
  * Main flipdot data structure
  * Passed as a pointer to most of the flipdot functions.
  */
typedef struct {
    /**
      * Used for inter task communication with the flipdot task
      */
    EventGroupHandle_t event_group;

    /**
      * @brief Flipdot rendering task
      * Performs the following actions, when `FLIPDOT_FRAMEBUFFER_DIRTY_BIT` in `event_group` was set:
      * * Delete `framebuffer_internal_old`
      * * Copy `framebuffer_internal` to `framebuffer_internal_old`
      * * Delete `framebuffer_internal`
      * * Copy `framebuffer` to `framebuffer_internal`
      * * Delete `internal_rendering_options`
      * * Copy `rendering_options` to `internal_rendering_options`
      * * Render the flipdot as configured
      */
    TaskHandle_t* task;

    /**
      * You can manipulate this framebuffer as you please.
      * Once done, call `flipdot_set_dirty_flag`
      * @see flipdot_set_dirty_flag
      */
    framebuffer_t* framebuffer;

    /**
      * Internal framebuffer, used to avoid race conditions when rendering.
      */
    framebuffer_t* framebuffer_internal;

    /**
      * Previous internal framebuffer, used for differential rendering
      */
    framebuffer_t* framebuffer_internal_old;

    /**
      * Panel configuration
      */
    flipdot_panel_t* panels;

    /**
      * Number of attached panels
      */
    uint8_t panel_count;

    /**
      * Total flipdot width in pixels
      */
    uint8_t width;
    
    /**
      * Rendering options
      */
    flipdot_rendering_options_t* rendering_options;

    /**
      * Internal rendering options, used internally to avoid race conditions when rendering.
      */
    flipdot_rendering_options_t* internal_rendering_options;

    /**
      * Flipdot power status
      * @note Use `flipdot_set_power` to manipulate this member
      * @see flipdot_set_power
      */
    bool power_status;

    /**
      * SPI device handle
      * @note SPI is used for controlling shift registers IC5, IC6 and IC7.
      */
    spi_device_handle_t spi_device_handle;

    /**
      * Flipdot GPIO state, used for shifting out IO states via SPI to IC5, IC6 and IC7.
      */
    flipdot_io_state_t io;

    /**
      * Counts the pixels that changed their color since startup
      */
    unsigned long long pixels_flipped;

    /**
      * Used for mutual exclusive hardware access
      */
    SemaphoreHandle_t semaphore;
} flipdot_t;

/**
  * Used to initialize a flipdot
  * @see flipdot_initialize
  */
typedef struct {
    /**
      * Number of attached panels
      */
    uint8_t panel_count;
    /**
      * Width of the attached panels
      */
    uint8_t panel_size[FLIPDOT_MAX_SUPPORTED_PANELS];
    /**
    * Default rendering mode
    */
    flipdot_rendering_mode_t rendering_mode;
} __attribute__((packed)) flipdot_configuration_t;


/**
  * @brief Initializes a flipdot for use and starts the flipdot task
  *
  * @note To actually use the flipdot you have to power it up using flipdot_set_power.
  * @param flipdot Flipdot to initialize, must not be NULL
  * @param flipdot_configuration Configuration to apply, must not be NULL
  * @see flipdot_set_power
  * @return
  *   - `ESP_OK`: success
  *   - `ESP_ERR_INVALID_ARG`: invalid arguments (i.e. null pointers)
  *   - `ESP_ERR_NO_MEM`: allocating internal buffers failed
  *   - other: failure
  */
esp_err_t flipdot_initialize(flipdot_t* flipdot, flipdot_configuration_t* flipdot_configuration);

/**
  * @brief Power on / off the flipdot
  * @note This does not stop the rendering task
  * @param flipdot The flipdot handle
  * @param power_status
  *  - True power on the flipdot
  *  - False power off the flipdot
  * @return
  *   - ESP_OK success
  *   - ESP_TIMEOUT timeout waiting for flipdot->semaphore
  */
esp_err_t flipdot_set_power(flipdot_t* flipdot, bool power_status);

/**
  * @brief Returns if the flipdot is powered on
  * @param flipdot The flipdot handle
  */
bool flipdot_get_power(flipdot_t* flipdot);

/**
  * Notfies the flipdot task, that the framebuffer is dirty and needs redrawing
  */
esp_err_t flipdot_set_dirty_flag(flipdot_t* flipdot);
