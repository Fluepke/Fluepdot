#pragma once

#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
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
    uint8_t width;
    uint8_t x;
} flipdot_panel_t;

/**
  * Main flipdot data structure
  * Passed as a pointer to most of the flipdot functions.
  */
typedef struct {
    /**
      * Used for inter task communication by the @see task.
      */
    EventGroupHandle_t event_group;

    /**
      * @brief Flipdot rendering task
      * Performs the following action, once the framebuffer dirty bit was set on the @see event_group:
      * * Delete @see framebuffer_internal_old
      * * Copy @see framebuffer_internal to @see framebuffer_internal_old
      * * Delete @see framebuffer_internal
      * * Copy @see framebuffer to @see framebuffer_internal
      * * Delete @see internal_rendering_options
      * * Copy @see rendering_options to internal_rendering_options
      * * Render the flipdot as configured.
      */
    TaskHandle_t* task;

    /**
      * @brief Framebuffer
      * You can manipulate this framebuffer as you please.
      * Once done, call @see flipdot_set_dirty_flag
      */
    framebuffer_t* framebuffer;

    /**
      * @brief Internal framebuffer
      * Used internally to avoid race conditions when rendering.
      */
    framebuffer_t* framebuffer_internal;

    /**
      * @brief Previous internal framebuffer
      * Used for differential rendering
      */
    framebuffer_t* framebuffer_internal_old;

    /**
      * @brief Panel configuration
      */
    flipdot_panel_t* panels;

    /**
      * @brief Number of attached panels
      */
    uint8_t panel_count;

    /**
      * @brief Total flipdot width in pixels
      */
    uint8_t width;
    
    /**
      * @brief Rendering options
      */
    flipdot_rendering_options_t* rendering_options;

    /**
      * @brief Internal rendering options
      * Used internally to avoid race conditions when rendering.
      */
    flipdot_rendering_options_t* internal_rendering_options;

    /**
      * @brief Flipdot power supply status
      * @note Use @see flipdot_set_power to manipulate this member
      */
    bool power_status;

    /**
      * @brief SPI device handle
      * SPI is used for controlling shift registers IC5, IC6 and IC7.
      */
    spi_device_handle_t spi_device_handle;

    /**
      * @brief Flipdot GPIO state
      */
    flipdot_io_state_t io;

    /**
      * @brief Counts the pixels that changed their value since startup
      */
    unsigned long long pixels_flipped;

    /**
      * @brief Used for mutual exclusive hardware access
      */
    SemaphoreHandle_t semaphore;
} flipdot_t;

/**
  * flipdot_configuration_t is used to initialize a flipdot
  */
typedef struct {
    uint8_t panel_count;
    uint8_t panel_size[FLIPDOT_MAX_SUPPORTED_PANELS];
} __attribute__((packed)) flipdot_configuration_t;


/**
  * @brief Initializes a flipdot for use and starts the @see flipdot.task
  * Initializes required internal data structures.
  * @note To actually use the flipdot you have to power it up using @flipdot_set_power
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
bool flipdot_get_power(flipdot_t*);

/**
  * @brief Marks the flipdot to be dirty
  * The @see flipdot.task is notified to render the flipdot
  */
esp_err_t flipdot_set_dirty_flag(flipdot_t* flipdot);
