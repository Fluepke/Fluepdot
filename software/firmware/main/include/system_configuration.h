#pragma once

#include <stdint.h>
#include "esp_err.h"

#include "flipdot.h"

#define SYSTEMCONFIGURATION_PARTITION_TYPE (0x42)
#define SYSTEMCONFIGURATION_PARTITION_SUBTYPE (0x23)
#define SYSTEMCONFIGURATION_PARTITION_LABEL "config"

#define WIFI_MODES C(DISABLED)C(WIFI_AP)C(WIFI_STATION)
#define C(x) x,

typedef enum {
    WIFI_DISABLED = 0,
    WIFI_AP = 1,
    WIFI_STATION = 2
} system_configuration_wifi_mode_t; 

typedef struct {
    system_configuration_wifi_mode_t mode;
    char ssid[32];
    char password[64];
} __attribute__((packed)) system_configuration_wifi_t;

typedef enum {
   RS485_DISABLED,
   RS485_FLIPNET = 1,
   RS485_DMX = 2
} system_configuration_rs485_mode_t;

typedef struct {
    system_configuration_rs485_mode_t mode;
    uint8_t address;
    unsigned long baudrate;
} __attribute__((packed)) system_configuration_rs485_t;

typedef struct {
    char hostname[32];
    system_configuration_wifi_t wifi;
    system_configuration_rs485_t rs485;
    flipdot_configuration_t flipdot;
    uint16_t checksum;
} __attribute__((packed)) system_configuration_t;

esp_err_t system_configuration_load(system_configuration_t* system_configuration);
esp_err_t system_configuration_save(system_configuration_t* system_configuration);
void system_configuration_dump(system_configuration_t* system_configuration);
void system_configuration_load_defaults(system_configuration_t* system_configuration);
