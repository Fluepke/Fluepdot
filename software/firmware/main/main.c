#include "console.h"
#include "main.h"
#include "wifi.h"
#include "mdns_util.h"
#include "httpd.h"
#include "snmp.h"
#include "util.h"
#include "bluetooth.h"

#include "freertos/FreeRTOS.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char* TAG = "main.c";

system_configuration_t system_configuration;
wifi_t wifi;
flipdot_t flipdot;
httpd_handle_t httpd_server;


static esp_err_t initialize() {
    // initialize non-volatile storage
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ERROR_CHECK(err);

    // Initialize the TCP/IP stack.
    ERROR_CHECK(esp_netif_init());

    // Create default event loop
    ERROR_CHECK(esp_event_loop_create_default());

    return ESP_OK;
}

void app_main() {
    ESP_LOGI(TAG, "Flipdot firmware, git commit %s", GIT_SHA1);

    // global initializations
    ESP_ERROR_CHECK(initialize());

    // load the system configuration
    ESP_ERROR_CHECK(system_configuration_load(&system_configuration));
    ESP_LOGI(TAG, "System configuration loaded");

    // initialize wireless connections
    ERROR_SHOW(wifi_initialize(&wifi, &system_configuration.wifi, system_configuration.hostname));
    ESP_LOGI(TAG, "WiFi initialized");

    // initialize mDNS
    ERROR_SHOW(mdns_initialize(system_configuration.hostname));
    ESP_LOGI(TAG, "mDNS initialized");

    // initialize Bluetooth LE
    ERROR_SHOW(bluetooth_initialize());
    ESP_LOGI(TAG, "Bluetooth initialized");

    // TODO initialize wired connections

    // initialize the flipdot
    ERROR_SHOW(flipdot_initialize(&flipdot, &system_configuration.flipdot));
    ESP_LOGI(TAG, "Flipdot initialized");

    // power on the flipdot
    ERROR_SHOW(flipdot_set_power(&flipdot, true));
    ESP_LOGI(TAG, "Flipdot powered up");

    // initialize the webserver
    ERROR_SHOW(httpd_initialize(httpd_server));
    ESP_LOGI(TAG, "httpd initialized");

    // initialize the SNMP agent
    ERROR_SHOW(snmp_initialize());
    ESP_LOGI(TAG, "snmp initialized");

    // intialize the interactive console
    ERROR_SHOW(console_initialize(&system_configuration));
    ESP_LOGI(TAG, "console initialized");

    while (true) {
        vTaskDelay(500 * portTICK_PERIOD_MS);
    }
}
