#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "flipdot.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "font_rendering.h"
#include "mf_font.h"

#include "flipnet.h"
#include "esp_err.h"

#define WLAN_SSID CONFIG_WLAN_SSID
#define WLAN_PSK CONFIG_WLAN_PSK

#define STR(x) #x

#define PORT 42
#define TAG "main"

#define FONT_NAME "ComicSans16"

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

const char* boot_msg = "Foobar 2342 Fnord";


void wifi_connect(){
    wifi_config_t cfg = {
        .sta = {
            .ssid = WLAN_SSID,
            .password = WLAN_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg) );
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    //esp_log_level_set("wifi", ESP_LOG_NONE); // disable wifi driver logging
    tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void print_wifi_ip(void *pvParam){
    printf("printWiFiIP task started \n");
    while(1) {
        xEventGroupWaitBits(wifi_event_group,CONNECTED_BIT,true,true,portMAX_DELAY);
        tcpip_adapter_ip_info_t ip_info;
        ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
        printf("IP :  %s\n", ip4addr_ntoa(&ip_info.ip));

        vTaskDelete(NULL);
    }
}

// static void udp_server_task(void *pvParameters)
// {
//     const char* fail_msg = "Give me exactly " STR(FLIPDOT_MAX_WIDTH*2) " bytes!";
//     char* ok_msg = "Turning your bits into sound";
// 
//     char rx_buffer[FLIPDOT_MAX_WIDTH * 2];
//     uint16_t framebuffer[FLIPDOT_MAX_WIDTH];
//     char addr_str[128];
//     int addr_family;
//     int ip_protocol;
// 
//     // initalize flipdot 
//     flipdot_t* flipdot = (flipdot_t*)pvParameters;
//     rendering_options_t* rendering_options;
//     ESP_ERROR_CHECK(flipdot_get_default_rendering_options(&rendering_options));
// 
//     while (1) {
//         struct sockaddr_in dest_addr;
//         dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
//         dest_addr.sin_family = AF_INET;
//         dest_addr.sin_port = htons(PORT);
//         addr_family = AF_INET;
//         ip_protocol = IPPROTO_IP;
//         inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
// 
//         int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
//         if (sock < 0) {
//             ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
//             break;
//         }
//         ESP_LOGI(TAG, "Socket created");
// 
//         int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//         if (err < 0) {
//             ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
//         }
//         ESP_LOGI(TAG, "Socket bound, port %d", PORT);
// 
//         while (1) {
// 
//             ESP_LOGI(TAG, "Waiting for data");
//             struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
//             socklen_t socklen = sizeof(source_addr);
//             int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer), 0, (struct sockaddr *)&source_addr, &socklen);
// 
//             // Error occurred during receiving
//             if (len < 0) {
//                 ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
//                 break;
//             }
//             // Data received
//             else {
//                 // Get the sender's ip address as string
//                 if (source_addr.sin6_family == PF_INET) {
//                     inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
//                 } else if (source_addr.sin6_family == PF_INET6) {
//                     inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
//                 }
// 
//                 ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
//                 //ESP_LOGI(TAG, "%s", rx_buffer);
// 
//                 int err;
//                 if (len == FLIPDOT_MAX_WIDTH * 2) {
//                     err = sendto(sock, ok_msg, strlen(ok_msg), 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
//                     memcpy(framebuffer, rx_buffer, FLIPDOT_MAX_WIDTH * 2);
//                     flipdot_render(flipdot, framebuffer, rendering_options);
//                 } else {
// 		    ESP_LOGE(TAG, "Expected %d bytes", FLIPDOT_MAX_WIDTH * 2);
//                     err = sendto(sock, fail_msg, strlen(fail_msg), 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
//                 }
//                 if (err < 0) {
//                     ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//                     break;
//                 }
//             }
//         }
// 
//         if (sock != -1) {
//             ESP_LOGE(TAG, "Shutting down socket and restarting...");
//             shutdown(sock, 0);
//             close(sock);
//         }
//     }
//     vTaskDelete(NULL);
// }

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
    flipdot_render_printf(flipdot, framebuffer);
    flipdot_render(flipdot, framebuffer, rendering_options);
}

/* Prints available fonts and example to the serial interface */
//void demo() {
//    const struct mf_font_list_s* font_list = mf_get_font_list();
//    while (font_list) {
//        const struct mf_font_s *font = font_list->font;
//        ESP_LOGI(TAG, "You are now about to experience %s", font->short_name);
//        state_t state = {};
//        state.width = FLIPDOT_MAX_WIDTH;
//        state.height = 16;
//        state.buffer = malloc(16 * FLIPDOT_MAX_WIDTH);
//        state.font = font;
//        state.y = -2;
//        memset(state.buffer, 255, 16 * FLIPDOT_MAX_WIDTH);
//        mf_render_justified(font, MF_ALIGN_CENTER, 0, FLIPDOT_MAX_WIDTH, boot_msg, 0, character_callback, &state);
//        ESP_LOGI(TAG, "Rendered.");
//        print_buffer(&state);
//        free(state.buffer);
//        font_list = font_list->next;
//    }
//}

/* Displays some text on the flipdot and scrolls this text if necesary */
void print_string(flipdot_t* flipdot, struct mf_font_s font, char* text) {
    int width;
}

static flipnet_interface_t interface;

/**
 * Simple diagnostics for the flipnet interface
 */
//void interface_diagnostics_task(void* param) {
//    flipnet_interface_t* interface;
//    for (;;) {
//        vTaskDelay(1000 / portTICK_PERIOD_MS);
//        ESP_LOGI(TAG, "interface stats: rx %d, drop rx %d",
//                interface->rx_count,
//                interface->rx_drop_count);
//    }
//}

/**
 * Simple consumer for frames received via the flipnet interface
 */
void receiver_task(void* param) {
    ESP_LOGI(TAG, "main receiver task started");
    flipnet_interface_t* interface = (flipnet_interface_t*)param;
    flipnet_frame_t frame;
    uint16_t framebuffer[115];
    bzero(framebuffer, 0);
    flipdot_t* flipdot;
    rendering_options_t* rendering_options;
    ESP_ERROR_CHECK(flipdot_init(&flipdot));
    ESP_ERROR_CHECK(flipdot_set_power(flipdot, true));
    ESP_ERROR_CHECK(flipdot_get_default_rendering_options(&rendering_options));

    for (;;) {
        if (!xQueueReceive(interface->rx_queue, (void*)&frame, (portTickType)portMAX_DELAY)) {
            continue;
        }
        switch (frame.command) {
            case FRAMEBUFFER:
                if (frame.payload_size != 230) {
                   ESP_LOGE(TAG, "cannot set any other framebuffer than 230 byte ones!");
                }
                ESP_LOGI(TAG, "framebuffer received");
                memcpy(framebuffer, frame.payload, 230);
                flipdot_render(flipdot, framebuffer, rendering_options);
                break;
            case RENDER:
                ESP_LOGI(TAG, "render command received");
                flipdot_render(flipdot, framebuffer, rendering_options);
                break;
            default:
                ESP_LOGI(TAG, "not yet implemented");
        }
        free(frame.payload);
        frame.payload = NULL;
    }
}

void app_main()
{
//    flipdot_t* flipdot;
//    rendering_options_t* rendering_options;
    ESP_ERROR_CHECK( nvs_flash_init() );
//    
//    // iterate font names
//    ESP_ERROR_CHECK(flipdot_init(&flipdot));
//    ESP_LOGI(TAG, "This flipdot has %d panels %d", flipdot->panel_count, FLIPDOT_PANEL_COUNT);
//    ESP_ERROR_CHECK(flipdot_get_default_rendering_options(&rendering_options));
//    ESP_ERROR_CHECK(flipdot_set_power(flipdot, true));
//
//    // initalize font rendering and print boot message
//    const struct mf_font_s *font;
//    font = mf_find_font(FONT_NAME);
//    if (!font) {
//        ESP_LOGE(TAG, "Font '%s' not found!", FONT_NAME);
//    }
//    ESP_LOGI(TAG, "This font's width %d", font->width);
//    state_t state = {};
//    state.width = FLIPDOT_MAX_WIDTH;
//    state.height = 16;
//    state.buffer = malloc(16 * FLIPDOT_MAX_WIDTH);
//    state.font = font;
//    state.y = 0;
//    if (state.buffer == NULL) {
//        ESP_LOGE(TAG, "Could not allocate buffer!");
//    }
//    memset(state.buffer, 255, 16 * FLIPDOT_MAX_WIDTH);
//
//    ESP_LOGI(TAG, "About to render some text");
//    mf_render_justified(font, MF_ALIGN_CENTER, -2, FLIPDOT_MAX_WIDTH, boot_msg, 0, character_callback, &state);
//    ESP_LOGI(TAG, "Rendered.");
//    print_buffer_flipdot(&state, flipdot, rendering_options);
    // start anything else 
//    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
//    wifi_event_group = xEventGroupCreate();
//    initialise_wifi();
//    xTaskCreate(&print_wifi_ip,"printWiFiIP",2048,NULL,5,NULL);
//
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Hello world!%d\n", portTICK_PERIOD_MS);
// demo();
//    xTaskCreate(udp_server_task, "udp_server", 8096, (void*)flipdot, 5, NULL);
    flipnet_mode_t mode = MASTER;
    flipnet_interface_config_t config = {
        .mtu = 250,
        .address = 42,
        .mode = mode,
        .baudrate = 250000,
        .promiscuous_mode = false
    };
    ESP_ERROR_CHECK(flipnet_init(&interface, &config));
    //xTaskCreate(receiver_task, "receiver_task", 8192, (void*)&interface, 12, NULL);
    for(;;) {
        vTaskDelay(1);
    }
}
