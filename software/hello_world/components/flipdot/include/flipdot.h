#pragma once

#include <stdio.h>
#include "driver/spi_master.h"

#define FLIPDOT_PWR_ON 21
#define FLIPDOT_SPI_MISO 12
#define FLIPDOT_SPI_MOSI 13
#define FLIPDOT_SPI_CLK 14
#define FLIPDOT_RCLK_PIN 19
#define FLIPDOT_SRCLR_PIN 18
#define FLIPDOT_OE_PIN 15

#define FLIPDOT_MAX_PANEL_COUNT 6
#define FLIPDOT_PANEL_COUNT CONFIG_FLIPDOT_PANEL_COUNT
#define PANEL0_WIDTH CONFIG_FLIPDOT_PANEL0_WIDTH
#define PANEL1_WIDTH CONFIG_FLIPDOT_PANEL1_WIDTH
#define PANEL2_WIDTH CONFIG_FLIPDOT_PANEL2_WIDTH
#define PANEL3_WIDTH CONFIG_FLIPDOT_PANEL3_WIDTH
#define PANEL4_WIDTH CONFIG_FLIPDOT_PANEL4_WIDTH
#define PANEL5_WIDTH CONFIG_FLIPDOT_PANEL5_WIDTH

#define FLIPDOT_MAX_WIDTH (PANEL0_WIDTH + PANEL1_WIDTH + PANEL2_WIDTH + PANEL3_WIDTH + PANEL4_WIDTH + PANEL5_WIDTH)

#define FLIPDOT_CLEAR_TIME CONFIG_FLIPDOT_DEFAULT_CLEARTIME
#define FLIPDOT_ON_TIME CONFIG_FLIPDOT_DEFAULT_ONTIME

/**
 * IO states of the flipdot driver.
 * Used for writing to the shift registers.
 */
typedef struct {
    bool clock_signal; /*!< true represents a 12V signal on the clock line */
    bool reset_signal; /*!< true represents a 12V signal on the reset line */
    bool clear_signal; /*!< true turns on the low side driver for the clear line */
    uint8_t panel_select; /*!< which panel to select. Ranges from 0 (=none) to 6. */
    uint16_t column; /*!< state of the high side drivers. */
} io_state_t;

/**
 * Main flipdot datastructure
 */
typedef struct {
    spi_device_handle_t spi_device_handle; /*!< SPI device handle for writing to shift registers */
    io_state_t io_state; /*!< IO state used for shifting out to the registers */
    uint8_t panel_widths[FLIPDOT_MAX_PANEL_COUNT]; /*!< Width of each panel */
    uint8_t panel_count; /*!< Number of panels attached to the flipdot */
    bool power_status; /*!< Used to indicate if the flipdot was turned on / off */
} flipdot_t;

/**
 * Column rendering options
 */
typedef struct {
    bool skip_clear; /*!< Whether to skip the clear cycle */
    bool skip_column; /*!< Whether to skip rendering the column */
    uint32_t clear_time; /*!< Microseconds of the clear cycle */
    uint32_t on_time; /*!< Microseconds of the rendering cycle */
} column_rendering_options_t;

/**
 * Flipdot rendering options
 */
typedef struct {
    uint8_t panel_order[FLIPDOT_MAX_PANEL_COUNT]; /*!< Order of the panels */
    column_rendering_options_t column_rendering_options[FLIPDOT_MAX_WIDTH]; /*!< Per column options */
} rendering_options_t;

/**
 * @brief Initializes flipdot
 * Initializes the flipdot data structure, GPIO and SPI.
 *
 * @param flipdot Pointer pointer to a flipdot datastructure.
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_NO_MEM Failure
 */
esp_err_t flipdot_init(flipdot_t** flipdot);

/**
 * @brief Initializes a rendering_options_t struct with sane defaults.
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_NO_MEM Failure
 */
esp_err_t flipdot_get_default_rendering_options(rendering_options_t** rendering_options);

/**
 * @brief Initializes the SPI for use with the flipdot
 * @param flipdot Flipdot handle
 * @return
 */
esp_err_t flipdot_init_spi(flipdot_t* flipdot);

/**
 * @brief Initializes GPIO pins for use with the flipdot
 * @param flipdot Flipdot handle
 * @return
 */
esp_err_t flipdot_init_gpio(flipdot_t* flipdot);

/**
 * @brief Control flipdot's power supply
 *
 * After calling @see flipdot_init one may safely power up the flipdot.
 * DO NOT POWER ON THE FLIPDOT WHILE ONLY POWERSUPPLY IS VBUS OF THE USB INTERFACE!
 * @param flipdot Flipdot handle
 * @param power_on Whether to power on or off the flipdot
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG GPIO number error
 */
esp_err_t flipdot_set_power(flipdot_t* flipdot, bool power_on);

/**
 * @brief Writes flipdot->io_state to the shift registers
 * @param flipdot Flipdot handle
 * @return
 */
esp_err_t flipdot_write_registers(flipdot_t* flipdot);

/**
 * @brief Renders a framebuffer to the flipdot without any special options
 * @param flipdot Flipdot handle
 * @param framebuffer The framebuffer to render
 * @return
 */
esp_err_t flipdot_render(flipdot_t* flipdot, uint16_t* framebuffer, rendering_options_t* rendering_options);

/**
 * Renders a framebuffer on a previously selected @see flipdot_select_panel panel.
 * @brief Renders a single panel
 * @param flipdot Flipdot handle
 * @param panel The panel to render
 * @param framebuffer Framebuffer of the entire display, offset of panel is calculated.
 * @param rendering_options Rendering options
 * @return
 */
esp_err_t flipdot_render_panel(
        flipdot_t* flipdot, uint8_t panel, uint16_t* framebuffer, rendering_options_t* rendering_options);

/**
 * @brief Renders a single column
 * @param flipdot Flipdot handle
 * @param column Pixel value #TODO write down mapping of bits
 * @param column_rendering_options Rendering options for this column
 * @return
 */
esp_err_t flipdot_render_column(flipdot_t* flipdot, uint16_t column,
        column_rendering_options_t* column_rendering_options);

/**
 * Selects and initalizes a panel for rendering.
 * To select panel with number 5, special wiring on the debug port, which exposes `SELECT5` (`QH'` of the 3rd shift register).
 * @param flipdot Flipdot handle
 * @param panel The panel to select. Range is from 0 to 5. Zero-index based.
 * @note Only one panel can be selected at a time due to electrical constraints. Selecting a panel will deselect others.
 */
esp_err_t flipdot_select_panel(flipdot_t* flipdot, uint8_t panel);

/**
 * @brief Sets a pixel to the given value in a framebuffer
 * @param x x
 * @param y y
 * @param state The state to set
 * @param framebuffer The framebuffer to manipulate
 * @note Origin of the coordinate system is in the top left corner
 */
esp_err_t flipdot_set_pixel(uint16_t x, uint8_t y, bool state, uint16_t* framebuffer);
