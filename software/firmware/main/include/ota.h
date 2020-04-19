#pragma once

#include <string.h>
#include "esp_http_client.h"
#include "esp_err.h"

/**
 * Waits for WiFi to connect and then performs a firmware upgrade
 */
void simple_ota_task(void* pvParams);

/**
 * Connect to a WiFi
 */
void wifi_init_sta(uint8_t* ssid, uint8_t* psk);

esp_err_t http_event_handler(esp_http_client_event_t *evt);
