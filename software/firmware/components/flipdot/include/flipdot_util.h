#pragma once

#include "esp_log.h"

#define FLIPDOT_LOGE(tag, descr) { \
    ESP_LOGE(TAG, "Exception %s(%d): %s", __FUNCTION__, __LINE__, descr); \
}

#define FLIPDOT_ERROR_CHECK(func) { \
    esp_err_t error_code; \
    if ((error_code = func) != ESP_OK) { \
        ESP_LOGE(TAG,"Exception %s(%d)", __FUNCTION__, __LINE__); \
        return error_code; \
    }\
}

#define FLIPDOT_ERROR_SHOW(error, func) { \
    error = func; \
    if (error != ESP_OK) { \
        ESP_LOGE(TAG,"Exception %s(%d): %d", __FUNCTION__, __LINE__, error); \
    } \
}

#define FLIPDOT_ERROR_NO_PREV(error, func) { \
    if (error == ESP_OK) { \
        error = func; \
        if (error != ESP_OK) { \
            ESP_LOGE(TAG,"Exception %s(%d)", __FUNCTION__, __LINE__); \
        } \
    }\
}

#define FLIPDOT_ASSERT_NOT_NULL(test_val, return_val) \
    if ((test_val) == NULL) { \
        ESP_LOGE(TAG, "nullpointer exception in function %s:%d", \
                __FUNCTION__, \
                __LINE__); \
        return return_val; }

#define MIN(a,b) \
       ({ __typeof__ (a) _a = (a); \
           __typeof__ (b) _b = (b); \
         _a < _b ? _a : _b; })
