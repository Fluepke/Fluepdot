#pragma once

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_netif.h"

// Number of clients, that are allowed to connect if the ESP is configured as an AP
#define WIFI_MAX_CONNECTION (5)
// Number of retries to attempt if connecting to a WiFi failed
#define WIFI_MAXIMUM_RETRY (3)
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

typedef struct {
    esp_netif_t* netif;
    EventGroupHandle_t event_group;
    unsigned int retry_count;
} wifi_t;

/**
  * @brief Configure and activate wireless networking
  *
  * Configures the wireless network interface based on the given configuration.
  * If configured in station mode, tries to connect @see WIFI_MAXIMUM_RETRY times to the network.
  * This function call is blocking.
  *
  * @param wifi The @see wifi_t handle
  * @param system_configuration_wifi WiFi configuration to apply
  * @param hostname Hostname to configure on the wireless interface
  */
esp_err_t wifi_initialize(wifi_t* wifi, system_configuration_wifi_t* system_configuration_wifi, const char* hostname);
esp_err_t wifi_deinitialize(wifi_t* wifi, system_configuration_wifi_t* system_configuration_wifi);
