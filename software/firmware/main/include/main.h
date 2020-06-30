#pragma once

#include "flipdot.h"
#include "system_configuration.h"
#include "wifi.h"

#define ERROR_SHOW(func) { \
    esp_err_t error = func; \
    if (error != ESP_OK) { \
        ESP_LOGE(TAG,"Exception %s(%d): %d", __FUNCTION__, __LINE__, error); \
    } \
}

extern system_configuration_t system_configuration;
extern flipdot_t flipdot;
extern wifi_t wifi;
