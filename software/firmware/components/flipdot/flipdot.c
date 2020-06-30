#include "flipdot.h"

#include "driver/gpio.h"
#include <string.h>

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

static const char* TAG = "flipdot.c";

static esp_err_t flipdot_initialize_gpio(flipdot_t* flipdot);
static esp_err_t flipdot_write_registers(flipdot_t* flipdot);
static esp_err_t flipdot_cycle_internal_datastructures(flipdot_t* flipdot);
static esp_err_t flipdot_render_panel(flipdot_t* flipdot, uint8_t panel_index);
static esp_err_t flipdot_select_panel(flipdot_t* flipdot, uint8_t panel_index);
static esp_err_t flipdot_render_column(flipdot_t* flipdot, uint8_t x);
static void flipdot_task(void* param);

static esp_err_t flipdot_initialize_gpio(flipdot_t* flipdot) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 
            (1ULL << FLIPDOT_GPIO_RCLK_PIN) |
            (1ULL << FLIPDOT_GPIO_SRCLR_PIN) |
            (1ULL << FLIPDOT_GPIO_OE_PIN) |
            (1ULL << FLIPDOT_GPIO_PWR_ON),
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    FLIPDOT_ERROR_CHECK(gpio_config(&io_conf));

    return ESP_OK;
}

static esp_err_t flipdot_initialize_spi(flipdot_t* flipdot) {
    FLIPDOT_ASSERT_NOT_NULL(flipdot, ESP_ERR_INVALID_ARG);

    spi_bus_config_t bus_config = {
        .miso_io_num=FLIPDOT_GPIO_SPI_MISO,
        .mosi_io_num=FLIPDOT_GPIO_SPI_MOSI,
        .sclk_io_num=FLIPDOT_GPIO_SPI_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=3
    };
    spi_device_interface_config_t device_config = {
        .clock_speed_hz=FLIPDOT_SPI_SPEED_HZ,
        .mode=0,
        .spics_io_num=0,
        .queue_size=7,
    };
    FLIPDOT_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &bus_config, 1));
    FLIPDOT_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &device_config, &(flipdot->spi_device_handle)));

    return ESP_OK;
}

static esp_err_t flipdot_write_registers(flipdot_t* flipdot) {
    FLIPDOT_ASSERT_NOT_NULL(flipdot, ESP_ERR_INVALID_ARG);

    if (flipdot->io.clear && flipdot->io.rows) {
        /**
          * Powering column low side driver and row high side driver at once is forbidden.
          * It will blow F1.
          */
        ESP_LOGE(TAG, "Protected from blowing Fuse F1. Please file a bug report.");
        return ESP_ERR_INVALID_ARG;
    }

    // clear shift register
    FLIPDOT_ERROR_CHECK(gpio_set_level(FLIPDOT_GPIO_SRCLR_PIN, 0));
    ets_delay_us(100);
    FLIPDOT_ERROR_CHECK(gpio_set_level(FLIPDOT_GPIO_SRCLR_PIN, 1));

    spi_transaction_t spi_transaction;
    memset(&spi_transaction, 0, sizeof(spi_transaction));
    spi_transaction.flags = SPI_TRANS_USE_TXDATA;
    spi_transaction.length = 24;
    assert(sizeof(flipdot_io_state_t) == 3);
    memcpy(&spi_transaction.tx_data, &(flipdot->io), sizeof(flipdot_io_state_t));
    spi_transaction.user = (void*)0;
    FLIPDOT_ERROR_CHECK(spi_device_polling_transmit(flipdot->spi_device_handle, &spi_transaction));

    gpio_set_level(FLIPDOT_GPIO_RCLK_PIN, 0);
    ets_delay_us(100);
    gpio_set_level(FLIPDOT_GPIO_RCLK_PIN, 1);
    
    ets_delay_us(100);

    return ESP_OK;
}

esp_err_t flipdot_initialize(flipdot_t* flipdot, flipdot_configuration_t* flipdot_configuration) {
    FLIPDOT_ASSERT_NOT_NULL(flipdot, ESP_ERR_INVALID_ARG);
    FLIPDOT_ASSERT_NOT_NULL(flipdot_configuration, ESP_ERR_INVALID_ARG);

    flipdot->pixels_flipped = 0;
    flipdot->width = 0;
    flipdot->panel_count = flipdot_configuration->panel_count;
    flipdot->panels = calloc(flipdot_configuration->panel_count, sizeof(flipdot_panel_t));
    bzero(&(flipdot->io), sizeof(flipdot_io_state_t));

    FLIPDOT_ASSERT_NOT_NULL(flipdot->panels, ESP_ERR_NO_MEM);

    for (uint8_t i=0; i<flipdot->panel_count; i++) {
        flipdot->panels[i].width = flipdot_configuration->panel_size[i];
        flipdot->panels[i].x = flipdot->width;
        flipdot->width += flipdot_configuration->panel_size[i];
    }

    esp_err_t error;

    flipdot->event_group = xEventGroupCreate();
    error = (flipdot->event_group == NULL) ? ESP_ERR_NO_MEM : ESP_OK;

    if (error == ESP_OK) {
        flipdot->framebuffer = calloc(1, sizeof(framebuffer_t));
        if (flipdot->framebuffer == NULL) {
            ESP_LOGE(TAG, "Failed to allocate memory for framebuffer");
            error = ESP_ERR_NO_MEM;
        } else {
            error = flipdot_framebuffer_init(flipdot->framebuffer, flipdot->width);
        }
    }

    if (error == ESP_OK) {
        flipdot->framebuffer_internal = calloc(1, sizeof(framebuffer_t));
        if (flipdot->framebuffer_internal == NULL) {
            error = ESP_ERR_NO_MEM;
        } else {
            error = flipdot_framebuffer_init(flipdot->framebuffer_internal, flipdot->width);
        }
    }
   
    if (error == ESP_OK) {
        flipdot->rendering_options = calloc(1, sizeof(flipdot_rendering_options_t));
        if (flipdot->rendering_options == NULL) {
            ESP_LOGE(TAG, "Failed to allocate memory for rendering options");
            error = ESP_ERR_NO_MEM;
        } else {
            error  = flipdot_rendering_options_initialize(flipdot->rendering_options, flipdot->panel_count, flipdot->width);
        }
    }
    
    FLIPDOT_ERROR_NO_PREV(error,
        flipdot_initialize_spi(flipdot));
    
    FLIPDOT_ERROR_NO_PREV(error,
        flipdot_initialize_gpio(flipdot));
   
    if (error == ESP_OK) {
        flipdot->io.select = FLIPDOT_NO_SELECT;
        flipdot->io.clock = 1;
        flipdot->io.reset = 1;
        flipdot->io.clear = 0;
        flipdot->io.rows = 0;
        error = flipdot_write_registers(flipdot);
    }

    FLIPDOT_ERROR_NO_PREV(error,
        gpio_set_level(FLIPDOT_GPIO_OE_PIN, 0));
    
    FLIPDOT_ERROR_NO_PREV(error,
        gpio_set_level(FLIPDOT_GPIO_SRCLR_PIN, 1));
   
    if (error == ESP_OK) {
        flipdot->semaphore = xSemaphoreCreateMutex();
        error = (flipdot->semaphore == NULL) ? ESP_FAIL : ESP_OK;
    }

    if (error == ESP_OK) {
        error = (xTaskCreate(
                flipdot_task,
                "flipdot_task",
                12288,
                (void*)flipdot,
                8,
                flipdot->task) == pdPASS ? ESP_OK : ESP_FAIL);
    }
    
    if (error != ESP_OK) {
        if (flipdot->event_group != NULL) {
            vEventGroupDelete(flipdot->event_group);
            flipdot->event_group = NULL;
        }
        if (flipdot->panels != NULL) {
            free(flipdot->panels);
            flipdot->panels = NULL;
        }
        if (flipdot->framebuffer != NULL) {
            flipdot_framebuffer_free(flipdot->framebuffer);
            flipdot->framebuffer = NULL;
        }
        if (flipdot->framebuffer_internal != NULL) {
            flipdot_framebuffer_free(flipdot->framebuffer_internal);
            flipdot->framebuffer_internal = NULL;
        }
        if (flipdot->rendering_options != NULL) {
            free(flipdot->rendering_options);
            flipdot->rendering_options = NULL;
        }
    }

    return error;
}

esp_err_t flipdot_set_power(flipdot_t* flipdot, bool power_status) {
    if (!xSemaphoreTake(flipdot->semaphore, portMAX_DELAY)) {
        return ESP_ERR_TIMEOUT;
    }
    esp_err_t error = gpio_set_level(FLIPDOT_GPIO_PWR_ON, power_status);
    if (error == ESP_OK) {
        flipdot->power_status = power_status;
    }

    xSemaphoreGive(flipdot->semaphore);

    return error;
}

bool flipdot_get_power(flipdot_t* flipdot) {
    return flipdot->power_status;
}

esp_err_t flipdot_render(flipdot_t* flipdot) {
    FLIPDOT_ASSERT_NOT_NULL(flipdot, ESP_ERR_INVALID_ARG);
    FLIPDOT_ASSERT_NOT_NULL(flipdot->framebuffer, ESP_ERR_INVALID_ARG);
    FLIPDOT_ASSERT_NOT_NULL(flipdot->rendering_options, ESP_ERR_INVALID_ARG);

    if (!xSemaphoreTake(flipdot->semaphore, portMAX_DELAY)) {
        ESP_LOGE(TAG, "Timeout waiting for flipdot->semaphore");
        return ESP_ERR_TIMEOUT;
    }

    if (!flipdot_get_power(flipdot)) {
        xSemaphoreGive(flipdot->semaphore);
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t error;
    FLIPDOT_ERROR_SHOW(error,
        flipdot_cycle_internal_datastructures(flipdot));
    if (error != ESP_OK) {
        xSemaphoreGive(flipdot->semaphore);
        return error;
    }

    // diff internal framebuffer against old internal framebuffer
    unsigned int pixels_changed = 0;
    FLIPDOT_ERROR_SHOW(error,
        flipdot_framebuffer_compare(
            flipdot->framebuffer_internal, flipdot->framebuffer_internal_old, &pixels_changed));

    if (error != ESP_OK) {
        xSemaphoreGive(flipdot->semaphore);
        return error;
    }

    if ((flipdot->internal_rendering_options->mode == DIFFERENTIAL)
            && (pixels_changed == 0)) {
        xSemaphoreGive(flipdot->semaphore);
        return ESP_OK; 
    }

    uint8_t current_panel_index;
    flipdot_panel_t* current_panel;

    for (uint8_t i=0; i<flipdot->panel_count; i++) {
        current_panel_index = flipdot->internal_rendering_options->panel_order[i];
        current_panel = &flipdot->panels[current_panel_index];

        FLIPDOT_ERROR_CHECK(
                flipdot_framebuffer_compare_partial(
                    flipdot->framebuffer_internal,
                    flipdot->framebuffer_internal_old,
                    current_panel->x,
                    current_panel->width,
                    &pixels_changed));


        if ((flipdot->internal_rendering_options->mode == DIFFERENTIAL)
                && (pixels_changed == 0)) {
            ESP_LOGV(TAG, "Skipping panel %d, due to rendering mode differential and no changes", current_panel_index);
            continue;
        }

        FLIPDOT_ERROR_CHECK(flipdot_render_panel(flipdot, current_panel_index));
    }
 
    xSemaphoreGive(flipdot->semaphore);

    return ESP_OK;
}

static esp_err_t flipdot_cycle_internal_datastructures(flipdot_t* flipdot) {
    // copy rendering options to internal structures
    flipdot_rendering_options_free(flipdot->internal_rendering_options);
    flipdot->internal_rendering_options = calloc(1, sizeof(flipdot_rendering_options_t));
    FLIPDOT_ASSERT_NOT_NULL(flipdot->internal_rendering_options, ESP_ERR_NO_MEM);
    FLIPDOT_ERROR_CHECK(flipdot_rendering_options_copy(flipdot->internal_rendering_options, flipdot->rendering_options));

    // copy internal framebuffer to old internal framebuffer
    flipdot_framebuffer_free(flipdot->framebuffer_internal_old);
    flipdot->framebuffer_internal_old = calloc(1, sizeof(framebuffer_t));
    FLIPDOT_ASSERT_NOT_NULL(flipdot->framebuffer_internal_old, ESP_ERR_NO_MEM);
    FLIPDOT_ERROR_CHECK(flipdot_framebuffer_copy(flipdot->framebuffer_internal_old, flipdot->framebuffer_internal));

    // copy framebuffer to internal framebuffer
    flipdot_framebuffer_free(flipdot->framebuffer_internal);
    flipdot->framebuffer_internal = calloc(1, sizeof(flipdot_rendering_options_t));
    FLIPDOT_ASSERT_NOT_NULL(flipdot->framebuffer_internal, ESP_ERR_NO_MEM);
    FLIPDOT_ERROR_CHECK(flipdot_framebuffer_copy(flipdot->framebuffer_internal, flipdot->framebuffer));
  
    return ESP_OK;
}

static esp_err_t flipdot_render_panel(flipdot_t* flipdot, uint8_t panel_index) {
    ESP_LOGV(TAG, "flipdot_render_panel %d", panel_index);
    FLIPDOT_ASSERT_NOT_NULL(flipdot, ESP_ERR_INVALID_ARG);
    FLIPDOT_ASSERT_NOT_NULL(flipdot->framebuffer_internal, ESP_ERR_INVALID_ARG);
    FLIPDOT_ASSERT_NOT_NULL(flipdot->internal_rendering_options, ESP_ERR_INVALID_ARG);

    FLIPDOT_ERROR_CHECK(flipdot_select_panel(flipdot, panel_index));
    
    flipdot_panel_t* panel = &flipdot->panels[panel_index];

    for (uint8_t x=0; x<panel->width; x++) {
        FLIPDOT_ERROR_CHECK(
                flipdot_render_column(
                    flipdot,
                    panel->x + x));
    }

    return ESP_OK;
}

static esp_err_t flipdot_select_panel(flipdot_t* flipdot, uint8_t panel_index) {
    ESP_LOGV(TAG, "flipdot_select_panel %d", panel_index);
    FLIPDOT_ASSERT_NOT_NULL(flipdot, ESP_ERR_INVALID_ARG);
    if (panel_index >= flipdot->panel_count) { return ESP_ERR_INVALID_ARG; }

    // perform reset
    flipdot->io.select = FLIPDOT_NO_SELECT;
    flipdot->io.clock = 1;
    flipdot->io.reset = 0;
    flipdot->io.clear = 0;
    flipdot->io.rows = 0;
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));
    ets_delay_us(100);

    // clock in inital 1
    flipdot->io.select = (~(1 << panel_index)) & 0b00011111;
    flipdot->io.reset = 1;

    // flipdot->io.control_register.reset = 0;
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));
    ets_delay_us(100);

    return ESP_OK;
}

static esp_err_t flipdot_render_column(flipdot_t* flipdot, uint8_t x) {
    ESP_LOGV(TAG, "flipdot_render_column %d", x);
    FLIPDOT_ASSERT_NOT_NULL(flipdot, ESP_ERR_INVALID_ARG);
    if (x >= flipdot->width) { return ESP_ERR_INVALID_ARG; }

    uint16_t column_old = flipdot->framebuffer_internal_old ->columns[x];
    uint16_t column = flipdot->framebuffer_internal->columns[x];
    unsigned int pixels_changed = __builtin_popcount(
            column_old ^ column);

    // old  0 0 1 1
    // new  0 1 0 1
    // need 0 0 1 0

    bool clear_needed = column_old & (~column);
    bool skip_clear = !clear_needed && (flipdot->internal_rendering_options->mode == DIFFERENTIAL);
    bool skip_set = (column == 0 && (flipdot->internal_rendering_options->mode == DIFFERENTIAL))
            || ((pixels_changed == 0) && (flipdot->internal_rendering_options->mode == DIFFERENTIAL));

    flipdot_rendering_delay_options_t* delays = 
        &(flipdot->internal_rendering_options->delay_options[x]);

    /** PRE CYCLE **/
    ets_delay_us(((uint32_t)delays->pre_delay) * 50);

    /** CLEAR CYCLE **/
    flipdot->io.clock = 0;
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));

    flipdot->io.select = FLIPDOT_NO_SELECT;
    flipdot->io.clock = 1;
    if (!skip_clear) {
        flipdot->io.clear = 1;
    }
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));

    if (!skip_clear) {
        ets_delay_us((uint32_t)(delays->clear_delay) * 50);
        flipdot->io.clear = 0;
        flipdot_write_registers(flipdot);
    } else {
        ets_delay_us(100);
    }

    /** SET CYCLE **/
    flipdot->io.clock = 0;
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));
    flipdot->io.clock = 1;
    flipdot->io.rows = flipdot->framebuffer_internal->columns[x];
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));

    if (!skip_set) {
        ets_delay_us((uint32_t)(delays->set_delay) * 50);
    } else {
        ets_delay_us(100);
    }
    flipdot->io.rows = 0;
    FLIPDOT_ERROR_CHECK(flipdot_write_registers(flipdot));

    flipdot->pixels_flipped += pixels_changed;

    return ESP_OK;
}

esp_err_t flipdot_set_dirty_flag(flipdot_t* flipdot) {
    FLIPDOT_ASSERT_NOT_NULL(flipdot, ESP_ERR_INVALID_ARG);
    
    xEventGroupSetBits(flipdot->event_group, FLIPDOT_FRAMEBUFFER_DIRTY_BIT);

    return ESP_OK;
}

static void flipdot_task(void* param) {
    if (param == NULL) {
        ESP_LOGE(TAG, "param must not be null in flipdot_task");
        return;
    }

    flipdot_t* flipdot = (flipdot_t*)param;

    ESP_LOGI(TAG, "flipdot_task started");

    while(true) {
        xEventGroupClearBits(flipdot->event_group,
                FLIPDOT_RENDERING_DONE_BIT);
        EventBits_t bits = xEventGroupWaitBits(
                flipdot->event_group,
                FLIPDOT_FRAMEBUFFER_DIRTY_BIT,
                true,
                false,
                portMAX_DELAY);

        if ((bits & FLIPDOT_FRAMEBUFFER_DIRTY_BIT) != FLIPDOT_FRAMEBUFFER_DIRTY_BIT) {
            continue;
        }

        ESP_LOGV(TAG, "Rendering ...");
        esp_err_t error;
        FLIPDOT_ERROR_SHOW(error, flipdot_render(flipdot));
        ESP_LOGV(TAG, "Rendering done");
        xEventGroupSetBits(flipdot->event_group,
                FLIPDOT_RENDERING_DONE_BIT);
    }
}
