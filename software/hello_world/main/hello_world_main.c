#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "flipdot.h"

#include "font_rendering.h"
#include "mf_font.h"

#include "flipnet.h"
#include "esp_err.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define FONT_NAME "DejaVuSans16"
#define FLIPNET_ADDRESS 42
#define BOOT_MSG "address "STR(FLIPNET_ADDRESS)

const char* TAG = "main";

typedef struct {
    flipnet_interface_t* interface;
    flipdot_t* flipdot;
} receiver_task_param_t;


/**
 * Simple diagnostics for the flipnet interface
 */
void interface_diagnostics_task(void* param) {
    flipnet_interface_t* interface = (flipnet_interface_t*)param;
    for (;;) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI("diagnostics", "interface stats: rx %ld, drop rx %ld\n"
                "buffer fulls: %ld\n"
                "parity errors: %ld\n"
                "frame errors: %ld\n"
                "fifo overflows: %ld\n"
                "breaks: %ld\n"
                "queue fulls: %ld\n",
                interface->rx_count,
                interface->rx_drop_count,
                interface->rx_drop_buffer_full,
                interface->rx_drop_parity,
                interface->rx_drop_frame_error,
                interface->rx_drop_fifo_ovf,
                interface->rx_break_count,
                interface->rx_drop_queue_full);
        ESP_LOGI("diagnostics", "free heap %d", xPortGetFreeHeapSize());
    }
}

/**
 * Displays a framebuffer from font rendering lib on flipdot
 */
void print_buffer_flipdot(state_t* state, flipdot_t* flipdot, rendering_options_t* rendering_options) {
    uint16_t framebuffer[FLIPDOT_MAX_WIDTH];
    memset(framebuffer, 0, FLIPDOT_MAX_WIDTH * 2);
    for (int16_t y=0; y<state->height && y<16; y++) {
        for (int16_t x=0; x<state->width && x<FLIPDOT_MAX_WIDTH; x++) {
            uint8_t value = state->buffer[y * state->width + x];
            if (value < 128) {
                framebuffer[x] |= (1 << y);
            }
        }
    }
    flipdot_render(flipdot, framebuffer, rendering_options);
    flipdot_render_printf(flipdot, framebuffer);
}


/**
 * Simple consumer for frames received via the flipnet interface
 */
void receiver_task(void* param) {
    ESP_LOGI(TAG, "main receiver task started");
    receiver_task_param_t* receiver_task_param = (receiver_task_param_t*)param;
    if (receiver_task_param->interface == NULL) {
        ESP_LOGE(TAG, "interface was null");
    }
    if (receiver_task_param->flipdot == NULL) {
        ESP_LOGE(TAG, "flipdot was null");
    }
    flipnet_interface_t* interface = receiver_task_param->interface;
    flipdot_t* flipdot = receiver_task_param->flipdot;

    flipnet_frame_t frame;
    rendering_options_t* rendering_options;
    ESP_ERROR_CHECK(flipdot_get_default_rendering_options(&rendering_options));
    uint16_t framebuffer[115];
    bzero(framebuffer, 0);
    struct mf_font_s* font;
    font = mf_find_font(FONT_NAME);
    state_t state = {
        .width = FLIPDOT_MAX_WIDTH,
        .height = 16,
        .buffer = malloc(16 * FLIPDOT_MAX_WIDTH),
        .font = font,
        .y = 0
    };

    for (;;) {
        if (!xQueueReceive(interface->rx_queue, (void*)&frame, (portTickType)portMAX_DELAY)) {
            continue;
        }
        switch (frame.command) {
            case POWER:
                if (frame.payload_size != 1) {
                    ESP_LOGE(TAG, "wring payload size");
                    break;
                }
                ESP_LOGI(TAG, "rcv. cmd: POWER");
                ESP_ERROR_CHECK(flipdot_set_power(flipdot, frame.payload[0]));
                break;
            case FRAMEBUFFER:
                ESP_LOGI(TAG, "rcv. cmd: FRAMEBUFFER");
                if (frame.payload_size != 230) {
                   ESP_LOGE(TAG, "wrong payload size");
                   break;
                }
                memcpy(framebuffer, frame.payload, 230);
                break;
            case RENDER:
                ESP_LOGI(TAG, "rcv. cmd: RENDER");
                flipdot_render(flipdot, framebuffer, rendering_options);
                flipdot_render_printf(flipdot, framebuffer);
                break;
            case CLEAR_OPTIONS:
                ESP_LOGI(TAG, "rcv. cmd: CLEAR_OPTIONS");
                for (int x=0; x<115; x++) {
                    rendering_options->column_rendering_options[x].clear_time =
                        (frame.payload[x * 2] << 8 | frame.payload[x * 2 + 1]) * 2;
                }
                break;
            case FLUEP_OPTIONS:
                ESP_LOGI(TAG, "rcv. cmd: FLUEP_OPTIONS");
                for (int x=0; x<115; x++) {
                    rendering_options->column_rendering_options[x].on_time =
                        (frame.payload[x * 2] << 8 | frame.payload[x * 2 + 1]) * 2;
                }
                break;
             case TEXT:
                memset(state.buffer, 255, 16 * FLIPDOT_MAX_WIDTH);
                ESP_LOGI(TAG, "font name is %s", font->full_name);
                mf_render_justified(font, MF_ALIGN_CENTER, -2, FLIPDOT_MAX_WIDTH, (char*)frame.payload, 0, character_callback, &state);
                print_buffer_flipdot(&state, flipdot, rendering_options);
                break;
             case LOAD_FONT:
                ESP_LOGI(TAG, "rcv. cmd: LOAD_FONT");
                font = mf_find_font((char*)frame.payload);
                if (!font) {
                    ESP_LOGE(TAG, "unable to load font %s", frame.payload);
                    font = mf_find_font(FONT_NAME);
                }
                state.font = font;
                break;
        }
        free(frame.payload);
    }
}

flipdot_t* flipdot = NULL;
flipnet_interface_t interface;

void app_main()
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    printf("Ahoy! My address is %d\n", FLIPNET_ADDRESS);
    printf("I am running version %s\n", GIT_SHA1);
    
    // setup flipnet interface
    flipnet_mode_t mode = SLAVE;
    flipnet_interface_config_t config = {
        .mtu = 250,
        .address = FLIPNET_ADDRESS,
        .mode = mode,
        .baudrate = 230400,
        .promiscuous_mode = false,
        .rx_queue_size = 23,
        .ignore_checksums = false
    };
    ESP_ERROR_CHECK(flipnet_init(&interface, &config));

    // setup the flipdot
    ESP_ERROR_CHECK(flipdot_init(&flipdot));

    // display the unit's address
     rendering_options_t* rendering_options;
     ESP_ERROR_CHECK(flipdot_get_default_rendering_options(&rendering_options));
     struct mf_font_s* font;
     font = mf_find_font(FONT_NAME);
     state_t state = {
         .width = FLIPDOT_MAX_WIDTH,
         .height = 16,
         .buffer = malloc(16 * FLIPDOT_MAX_WIDTH),
         .font = font,
         .y = 0
     };
     memset(state.buffer, 255, 16 * FLIPDOT_MAX_WIDTH);
     mf_render_justified(font, MF_ALIGN_CENTER, -2, FLIPDOT_MAX_WIDTH,
             BOOT_MSG, 0, character_callback, &state);
     ESP_ERROR_CHECK(flipdot_set_power(flipdot, true));
     print_buffer_flipdot(&state, flipdot, rendering_options);

    // create tasks
    receiver_task_param_t params = {
        .interface = &interface,
        .flipdot = flipdot
    };

    xTaskCreate(receiver_task, "receiver_task", 16384, (void*)&params, 12, NULL);
    xTaskCreate(interface_diagnostics_task, "diagnostics", 2048, (void*)&interface, 6, NULL);
    for(;;) {
        vTaskDelay(1);
    }
}
