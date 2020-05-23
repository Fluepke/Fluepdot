#include "esp_partition.h"
#include "esp_log.h"
#include "string.h"

#include "util.h"
#include "system_configuration.h"

static const char* TAG = "system_configuration.c";

const esp_partition_t* system_configuration_find_partition() {
    return esp_partition_find_first(
            SYSTEMCONFIGURATION_PARTITION_TYPE,
            SYSTEMCONFIGURATION_PARTITION_SUBTYPE,
            SYSTEMCONFIGURATION_PARTITION_LABEL
    );
}

static void system_configuration_load_defaults_wifi(system_configuration_t* system_configuration) {
#if CONFIG_DEFAULT_SYSTEMCONFIGURATION_WIFI_MODE_DISABLED
    system_configuration->wifi.mode = WIFI_DISABLED;
#elif CONFIG_DEFAULT_SYSTEMCONFIGURATION_WIFI_MODE_AP
    system_configuration->wifi.mode = WIFI_AP;
#elif CONFIG_DEFAULT_SYSTEMCONFIGURATION_WIFI_MODE_STATION
    system_configuration->wifi.mode = WIFI_STATION;
#endif
    bzero(system_configuration->wifi.ssid, sizeof(system_configuration->wifi.ssid));
    strncpy(system_configuration->wifi.ssid, CONFIG_DEFAULT_SYSTEMCONFIGURATION_WIFI_SSID,
            strnlen(CONFIG_DEFAULT_SYSTEMCONFIGURATION_WIFI_SSID, sizeof(system_configuration->wifi.ssid)));
    bzero(system_configuration->wifi.password, sizeof(system_configuration->wifi.password));
    strncpy(system_configuration->wifi.password, CONFIG_DEFAULT_SYSTEMCONFIGURATION_WIFI_PASSWORD,
            strnlen(CONFIG_DEFAULT_SYSTEMCONFIGURATION_WIFI_PASSWORD, sizeof(system_configuration->wifi.password)));
}

static void system_configuration_load_defaults_rs485(system_configuration_t* system_configuration) {
#if CONFIG_DEFAULT_SYSTEMCONFIGURATION_RS485_MODE_DISABLED
    system_configuration->rs485.mode = RS485_DISABLED;
#elif CONFIG_DEFAULT_SYSTEMCONFIGURATION_RS485_MODE_FLIPNET
    system_configuration->rs485.mode = RS485_FLIPNET;
    system_configuration->rs485.address = CONFIG_DEFAULT_SYSTEMCONFIGURATION_RS485_ADDRESS;
    system_configuration->rs485.baudrate = CONFIG_DEFAULT_SYSTEMCONFIGURATION_RS485_BAUDRATE;
#elif CONFIG_DEFAULT_SYSTEMCONFIGURATION_RS485_MODE_DMX
    system_configuration->rs485.mode = RS485_DMX;
#endif
}

static void system_configuration_load_defaults_flipdot(system_configuration_t* system_configuration) {
    system_configuration->flipdot.panel_count = CONFIG_DEFAULT_SYSTEMCONFIGURATION_FLIPDOT_PANEL_COUNT;
    system_configuration->flipdot.panel_size[0] = CONFIG_DEFAULT_SYSTEMCONFIGURATION_FLIPDOT_PANEL_SIZE1;
    system_configuration->flipdot.panel_size[1] = CONFIG_DEFAULT_SYSTEMCONFIGURATION_FLIPDOT_PANEL_SIZE2;
    system_configuration->flipdot.panel_size[2] = CONFIG_DEFAULT_SYSTEMCONFIGURATION_FLIPDOT_PANEL_SIZE3;
    system_configuration->flipdot.panel_size[3] = CONFIG_DEFAULT_SYSTEMCONFIGURATION_FLIPDOT_PANEL_SIZE4;
    system_configuration->flipdot.panel_size[4] = CONFIG_DEFAULT_SYSTEMCONFIGURATION_FLIPDOT_PANEL_SIZE5;
}

void system_configuration_load_defaults(system_configuration_t* system_configuration) {
    system_configuration_load_defaults_wifi(system_configuration);
    system_configuration_load_defaults_rs485(system_configuration);
    system_configuration_load_defaults_flipdot(system_configuration);

    bzero(system_configuration->hostname, sizeof(system_configuration->hostname));
    strncpy(system_configuration->hostname, CONFIG_DEFAULT_SYSTEMCONFIGURATION_HOSTNAME,
            strnlen(CONFIG_DEFAULT_SYSTEMCONFIGURATION_HOSTNAME, sizeof(system_configuration->hostname)));
}

static esp_err_t system_configuration_flash_defaults(system_configuration_t* system_configuration) {
    system_configuration_load_defaults(system_configuration);
    ERROR_CHECK(system_configuration_save(system_configuration));

    return ESP_OK;
}

esp_err_t system_configuration_load(system_configuration_t* system_configuration) {
    const esp_partition_t* config_partition = system_configuration_find_partition();
    if (config_partition == NULL) {
        return ESP_ERR_NOT_FOUND;
    }
    ERROR_CHECK(
        esp_partition_read(
            config_partition,
            0,
            (void*)system_configuration,
            sizeof(system_configuration_t)
        )
    );

    uint16_t checksum = system_configuration->checksum;
    system_configuration->checksum = 0;

    if (checksum != crc16((uint8_t*)system_configuration, sizeof(system_configuration_t))) {
        ESP_LOGE(TAG, "checksum verification failed, flashing defaults");
        return system_configuration_flash_defaults(system_configuration);
    }

    return ESP_OK;
}

esp_err_t system_configuration_save(system_configuration_t* system_configuration) {
    const esp_partition_t* config_partition = system_configuration_find_partition();
    if (config_partition == NULL) {
        return ESP_ERR_NOT_FOUND;
    }
    ERROR_CHECK(esp_partition_erase_range(config_partition, 0, config_partition->size));

    system_configuration->checksum = 0;
    uint16_t checksum = crc16((uint8_t*)system_configuration, sizeof(system_configuration_t));
    system_configuration->checksum = checksum;
    esp_err_t error = esp_partition_write(
            config_partition,
            0,
            (void*)system_configuration,
            sizeof(system_configuration_t)
    );
    system_configuration->checksum = 0;

    return error;
}

void system_configuration_dump(system_configuration_t* system_configuration) {
    printf("=== System Configuration ===\n");
    printf("hostname='%.32s'\n", system_configuration->hostname);
    printf("WiFi: mode=%d, ssid='%.32s', password='%.64s'\n",
            system_configuration->wifi.mode, system_configuration->wifi.ssid, system_configuration->wifi.password);
    printf("RS485: mode=%d, address=%d, baudrate=%ld\n",
            system_configuration->rs485.mode, system_configuration->rs485.address, system_configuration->rs485.baudrate);
    printf("Flipdot: panel_count=%d", system_configuration->flipdot.panel_count);
    for (int i=0; i<FLIPDOT_MAX_SUPPORTED_PANELS; i++) {
        printf(", panel_size[%d] = %d", i, system_configuration->flipdot.panel_size[i]);
    }
    printf("\n");
}
