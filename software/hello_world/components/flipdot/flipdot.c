#include "flipdot.h"
#include "esp_log.h"
#include "esp32/rom/ets_sys.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include <string.h>

static const char* FLIPDOT_TAG = "flipdot";

#define FLIPDOT_CHECK(a, str, ret_val, ...) \
    if (!(a)) { \
        ESP_LOGE(FLIPDOT_TAG,"%s(%d): "str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        return (ret_val); \
    }

#define FLIPDOT_ERROR_CHECK(func) { \
    esp_err_t error_code; \
    if ((error_code = func) != ESP_OK) { \
        ESP_LOGV(FLIPDOT_TAG,"Exception %s(%d)", __FUNCTION__, __LINE__); \
        return error_code; \
    }\
}

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

esp_err_t flipdot_init(flipdot_t** flipdot_handle) {
    flipdot_t *flipdot = malloc(sizeof(flipdot_t));
    if (flipdot == NULL) { return ESP_ERR_NO_MEM; }
    memset(flipdot, 0, sizeof(flipdot_t));

    flipdot->panel_count = FLIPDOT_PANEL_COUNT;
    flipdot->panel_widths[0] = PANEL0_WIDTH;
    flipdot->panel_widths[1] = PANEL1_WIDTH;
    flipdot->panel_widths[2] = PANEL2_WIDTH;
    flipdot->panel_widths[3] = PANEL3_WIDTH;
    flipdot->panel_widths[4] = PANEL4_WIDTH;
    flipdot->panel_widths[5] = PANEL5_WIDTH;

    FLIPDOT_ERROR_CHECK(flipdot_init_spi(flipdot));

    FLIPDOT_ERROR_CHECK(flipdot_init_gpio(flipdot));

    flipdot->io_state.reset_signal = 1;
	  FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));
    vTaskDelay(1);

    FLIPDOT_ERROR_CHECK(gpio_set_level(FLIPDOT_OE_PIN, 0))
    FLIPDOT_ERROR_CHECK(gpio_set_level(FLIPDOT_SRCLR_PIN, 1))
    *flipdot_handle = flipdot;

    return ESP_OK;
}

esp_err_t flipdot_get_default_rendering_options(rendering_options_t** rendering_options) {
    rendering_options_t *options = malloc(sizeof(rendering_options_t));
    if (options == NULL) { return ESP_ERR_NO_MEM; }
    memset(options, 0, sizeof(rendering_options_t));
    *rendering_options = options;

    for (int i = 0; i < FLIPDOT_PANEL_COUNT; i++) {
        options->panel_order[i] = i + 1;
    }
    for (int i=0; i < FLIPDOT_MAX_WIDTH; i++) {
        options->column_rendering_options[i].clear_time = FLIPDOT_CLEAR_TIME;
        options->column_rendering_options[i].on_time = FLIPDOT_ON_TIME;
    }
    return ESP_OK;
}

esp_err_t flipdot_set_power(flipdot_t* flipdot, bool power_on) {
    flipdot->power_status = power_on;
    return gpio_set_level(FLIPDOT_PWR_ON, power_on);
}

esp_err_t flipdot_init_spi(flipdot_t* flipdot) {
	spi_bus_config_t bus_config = {
        .miso_io_num=FLIPDOT_SPI_MISO,
        .mosi_io_num=FLIPDOT_SPI_MOSI,
        .sclk_io_num=FLIPDOT_SPI_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=3
    };
    spi_device_interface_config_t device_config = {
        .clock_speed_hz=1*1000*1000,  // here
        .mode=0,
        .spics_io_num=0,
        .queue_size=7,
    };
    FLIPDOT_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &bus_config, 1))
    FLIPDOT_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &device_config, &(flipdot->spi_device_handle)))
    ESP_LOGI(FLIPDOT_TAG, "SPI initialized.");
    return ESP_OK;
}

esp_err_t flipdot_init_gpio(flipdot_t* flipdot) {
	gpio_config_t io_conf = {
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << FLIPDOT_RCLK_PIN) |
            (1ULL << FLIPDOT_SRCLR_PIN) |
            (1ULL << FLIPDOT_OE_PIN) |
            (1ULL << FLIPDOT_PWR_ON),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    FLIPDOT_ERROR_CHECK(gpio_config(&io_conf))
    FLIPDOT_ERROR_CHECK(gpio_set_direction(FLIPDOT_SELECT6_PIN, GPIO_MODE_DEF_OUTPUT));
    FLIPDOT_ERROR_CHECK(gpio_set_level(FLIPDOT_SELECT6_PIN, true));
    ESP_LOGI(FLIPDOT_TAG, "GPIO initialized.");
    return ESP_OK;
}

esp_err_t flipdot_write_registers(flipdot_t* flipdot) {
    // clear shift register
    FLIPDOT_ERROR_CHECK(gpio_set_level(FLIPDOT_SRCLR_PIN, 0));
    ets_delay_us(100);
    FLIPDOT_ERROR_CHECK(gpio_set_level(FLIPDOT_SRCLR_PIN, 1));

	spi_transaction_t spi_transaction;
    memset(&spi_transaction, 0, sizeof(spi_transaction));
    spi_transaction.flags = SPI_TRANS_USE_TXDATA;
    spi_transaction.length = 32;
    // use QH' for controlling up to six panels
    FLIPDOT_ERROR_CHECK(gpio_set_level(FLIPDOT_SELECT6_PIN, flipdot->io_state.panel_select != 6));
    spi_transaction.tx_data[0] = flipdot->io_state.panel_select & (1 << 5) ? 1 : 0;
    spi_transaction.tx_data[1] =
    	(~((1 << flipdot->io_state.panel_select) >> 1) & 0b11111) |
    	(flipdot->io_state.clock_signal ? 0 : 1) << 5 |
    	(flipdot->io_state.reset_signal ? 0 : 1) << 6 |
    	flipdot->io_state.clear_signal << 7;
    spi_transaction.tx_data[2] = flipdot->io_state.column >> 8;
    spi_transaction.tx_data[3] = flipdot->io_state.column;
    spi_transaction.user = (void*)0;
    FLIPDOT_ERROR_CHECK(spi_device_polling_transmit(flipdot->spi_device_handle, &spi_transaction))
    gpio_set_level(FLIPDOT_RCLK_PIN, 0);
    ets_delay_us(100);
    gpio_set_level(FLIPDOT_RCLK_PIN, 1);
    ets_delay_us(100);

    return ESP_OK;
}

esp_err_t flipdot_render(flipdot_t* flipdot, uint16_t* framebuffer, rendering_options_t* rendering_options) {
    FLIPDOT_CHECK(flipdot->power_status, "Try turning it on first!", ESP_ERR_INVALID_STATE);
    for (int i=0; i<flipdot->panel_count; i++) {
        FLIPDOT_ERROR_CHECK(flipdot_render_panel(flipdot, i, framebuffer, rendering_options));
    }

    return ESP_OK;
}

esp_err_t flipdot_render_printf(flipdot_t* flipdot, uint16_t* framebuffer) {
    for (uint8_t y=0; y<15; y++) {
        for (uint8_t x=0; x<FLIPDOT_MAX_WIDTH; x++) {
            printf("%c", framebuffer[x] & (1 << y) ? 'X' : ' ');
        }
        printf("\n");
    }
    printf("\n");
    return ESP_OK;
}

esp_err_t flipdot_render_panel(
        flipdot_t* flipdot, uint8_t panel, uint16_t* framebuffer, rendering_options_t* rendering_options)
{
    // FLIPDOT_CHECK(flipdot->power_status, "Try turning it on first!", ESP_ERR_INVALID_STATE);
    int offset = 0;
    for (int i=0; i<panel; i++) {
        offset += flipdot->panel_widths[i];
    }

    ESP_LOGI(FLIPDOT_TAG, "Rendering to panel %d, offset (%d)", panel, offset);
    ESP_LOGV(FLIPDOT_TAG, "| 01234567 01234567 | clear | render |");
    FLIPDOT_ERROR_CHECK(flipdot_select_panel(flipdot, panel));
    for (int i=0; i<flipdot->panel_widths[panel]; i++) {
        FLIPDOT_ERROR_CHECK(
                flipdot_render_column(
                    flipdot, framebuffer[offset + i], &(rendering_options->column_rendering_options[i]))
                );
    }
    flipdot->io_state.reset_signal = 1;
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));
    return ESP_OK;
}

esp_err_t flipdot_select_panel(flipdot_t* flipdot, uint8_t panel) {
    FLIPDOT_CHECK(panel <= 5, "Can only select panel in range 0..5", ESP_ERR_INVALID_ARG);

    // perform reset
    flipdot->io_state.clock_signal = 0;
    flipdot->io_state.reset_signal = 1;
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));
    ets_delay_us(100);

    // clock in inital 1
    flipdot->io_state.panel_select = panel + 1;
    flipdot->io_state.reset_signal = 0;
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));
    ets_delay_us(100);

    return ESP_OK;
}

esp_err_t flipdot_render_column(
        flipdot_t* flipdot, uint16_t column, column_rendering_options_t* column_rendering_options)
{
    ESP_LOGV(FLIPDOT_TAG, "| "BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN" | %d | %d |",
            BYTE_TO_BINARY(column >> 8),
            BYTE_TO_BINARY(column),
            column_rendering_options->clear_time,
            column_rendering_options->on_time);

    flipdot->io_state.clock_signal = 1;
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));

    flipdot->io_state.panel_select = 0;

    flipdot->io_state.clock_signal = 0;
    if (column_rendering_options->clear_time > 0) {
        flipdot->io_state.clear_signal = 1;
    }
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));

    if (column_rendering_options->clear_time > 0) {
        ets_delay_us(column_rendering_options->clear_time);
        flipdot->io_state.clear_signal = 0;
        flipdot_write_registers(flipdot);
    }

    flipdot->io_state.clock_signal = 1;
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));

    flipdot->io_state.clock_signal = 0;
    if (column_rendering_options->on_time > 0) {
        flipdot->io_state.column = column;
    }
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));

    if (column_rendering_options->on_time > 0) {
        ets_delay_us(column_rendering_options->on_time);
        flipdot->io_state.column = 0;
        FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));
    }

    return ESP_OK;
}

esp_err_t flipdot_set_pixel(uint16_t x, uint8_t y, bool state, uint16_t* framebuffer) {
    framebuffer[x] ^= (-state ^ y) & (1UL << y);
    return ESP_OK;
}
