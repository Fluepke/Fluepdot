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

#define WLAN_SSID CONFIG_WLAN_SSID
#define WLAN_PSK CONFIG_WLAN_PSK

#define PORT 42
#define TAG "main"

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

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

static void udp_server_task(void *pvParameters)
{
    char* fail_msg = "Give me exactly 230 bytes!";
    char* ok_msg = "Turning your bits into sound";

    char rx_buffer[230];
    uint16_t framebuffer[FLIPDOT_MAX_WIDTH];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    flipdot_t* flipdot;
    rendering_options_t* rendering_options;
    ESP_ERROR_CHECK(flipdot_init(&flipdot));
    ESP_ERROR_CHECK(flipdot_get_default_rendering_options(&rendering_options));
    ESP_ERROR_CHECK(flipdot_set_power(flipdot, true));

    while (1) {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        while (1) {

            ESP_LOGI(TAG, "Waiting for data");
            struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer), 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                if (source_addr.sin6_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                } else if (source_addr.sin6_family == PF_INET6) {
                    inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                //ESP_LOGI(TAG, "%s", rx_buffer);

                int err;
                if (len == FLIPDOT_MAX_WIDTH * 2) {
                    err = sendto(sock, ok_msg, strlen(ok_msg), 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                    memcpy(framebuffer, rx_buffer, FLIPDOT_MAX_WIDTH * 2);
                    flipdot_render(flipdot, framebuffer, rendering_options);
                } else {
                    err = sendto(sock, fail_msg, strlen(fail_msg), 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                }
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}


void app_main()
{   
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_event_group = xEventGroupCreate();
    initialise_wifi();
    xTaskCreate(&print_wifi_ip,"printWiFiIP",2048,NULL,5,NULL);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    printf("Hello world!%d\n", portTICK_PERIOD_MS);

    xTaskCreate(udp_server_task, "udp_server", 8096, NULL, 5, NULL);
}
