#pragma once

#include "crc16.h"

#define ERROR_CHECK(func) { \
    esp_err_t error; \
    if (( error = (func)) != ESP_OK) { \
        ESP_LOGE(TAG, "Exception %s in function %s:%d occured", \
                esp_err_to_name(error), \
                __FUNCTION__, \
                __LINE__); \
        return error; \
    } \
}

#define ERROR_DISCARD(func) { \
    esp_err_t error; \
    if (( error = (func)) != ESP_OK) { \
        ESP_LOGE(TAG, "Exception %s in function %s:%d occured", \
                esp_err_to_name(error), \
                __FUNCTION__, \
                __LINE__); \
    } \
}

#define TIME_DIFF_MS(_end, _start) ((uint32_t)(((_end).tv_sec - (_start).tv_sec) * 1000 + \
    ((_end).tv_usec - (_start).tv_usec) / 1000))
