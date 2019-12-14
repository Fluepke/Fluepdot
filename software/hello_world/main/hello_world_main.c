#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "flipdot.h"

#include "font_rendering.h"
#include "diagnostics.h"
#include "ota.h"
#include "mf_font.h"

#include "flipnet.h"
#include "esp_err.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define FONT_NAME "DejaVuSans16"
#define BOOT_MSG "address "STR(FLIPNET_ADDRESS)
#define CONFIG_PARTITION_TYPE (0x42)
#define CONFIG_PARTITION_SUBTYPE (0x23)

const char* TAG = "main";

typedef struct {
    flipnet_interface_t* interface;
    flipdot_t* flipdot;
} receiver_task_param_t;

flipdot_t* flipdot = NULL;
flipnet_interface_t interface;

typedef struct {
    uint8_t address;
    uint32_t baudrate;
} __attribute__((packed)) system_configuration_t;

system_configuration_t system_configuration;

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

uint8_t ota_ssid[32] = {'\0'};
uint8_t ota_psk[64] = {'\0'};
char ota_update_url[255] = {'\0'};

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
             case OTA_WIFI_SSID:
                ESP_LOGI(TAG, "rcv. cmd: OTA_WIFI_SSID");
                if (frame.payload_size > 32) {
                    ESP_LOGE(TAG, "ssid max length 32 chars!");
                    break;
                }
                memcpy(ota_ssid, frame.payload, frame.payload_size);
                break;
            case OTA_WIFI_PSK:
                ESP_LOGI(TAG, "rcv. cmd: OTA_WIFI_PSK");
                if (frame.payload_size > 64) {
                    ESP_LOGE(TAG, "max psk length 64 chars!");
                    break;
                }
                memcpy(ota_psk, frame.payload, frame.payload_size);
                break;
            case OTA_UPDATE_URL:
                ESP_LOGI(TAG, "rcv. cmd: OTA_UDPATE_URL");
                memcpy(ota_update_url, frame.payload, frame.payload_size);
                break;
            case OTA_WIFI_CONNECT:
                ESP_LOGI(TAG, "rcv. cmd: OTA_WIFI_CONNECT");
                wifi_init_sta(ota_ssid, ota_psk);
                break;
            case OTA_UPDATE:
                ESP_LOGI(TAG, "rcv. cmd: OTA_UPDATE");
                xTaskCreate(&simple_ota_task, "simple_ota_task", 8192, (void*)ota_update_url, 5, NULL);
                break;
        }
        free(frame.payload);
    }
}


void app_main()
{
    printf("I am running version %s\n", GIT_SHA1);

    // initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // load configuration from the config partition
    const esp_partition_t* config_partition = esp_partition_find_first(
            CONFIG_PARTITION_TYPE,
            CONFIG_PARTITION_SUBTYPE,
            "config");
    if (config_partition == NULL) {
        ESP_LOGE(TAG, "could not locate configuration partition");
    }
    ESP_ERROR_CHECK((config_partition != NULL) ? ESP_OK : ESP_FAIL);
    ESP_ERROR_CHECK(
            esp_partition_read(
                config_partition, 0, (void*)&system_configuration, sizeof(system_configuration_t)));

    ESP_LOGI(TAG, "My address is %d and my baudrate is %d", system_configuration.address,
            system_configuration.baudrate);

    // setup flipnet interface
    flipnet_mode_t mode = SLAVE;
    flipnet_interface_config_t config = {
        .mtu = 250,
        .address = system_configuration.address,
        .mode = mode,
        .baudrate = system_configuration.baudrate,
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
//    xTaskCreate(interface_diagnostics_task, "diagnostics", 2048, (void*)&interface, 6, NULL);
    for(;;) {
        vTaskDelay(1);
    }
}
