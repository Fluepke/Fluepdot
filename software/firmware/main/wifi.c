#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "system_configuration.h"
#include "util.h"
#include "wifi.h"

static const char* TAG = "wifi.c";

static esp_err_t wifi_init_ap(wifi_t* wifi, system_configuration_wifi_t* system_configuration_wifi, const char* hostname);
static esp_err_t wifi_configure_ap(system_configuration_wifi_t* system_configuration_wifi);
static esp_err_t wifi_init(wifi_t* wifi);
static esp_err_t wifi_init_station(wifi_t* wifi, system_configuration_wifi_t* system_configuration_wifi, const char* hostname);
static esp_err_t wifi_configure_station(system_configuration_wifi_t* system_configuration_wifi);
static esp_err_t wifi_wait_connected(wifi_t* wifi);
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

esp_err_t wifi_initialize(wifi_t* wifi, system_configuration_wifi_t* system_configuration_wifi, const char* hostname) {
    switch (system_configuration_wifi->mode) {
        case WIFI_DISABLED:
            return ESP_OK;
        case WIFI_AP:
            return wifi_init_ap(wifi, system_configuration_wifi, hostname);
        case WIFI_STATION:
            return wifi_init_station(wifi, system_configuration_wifi, hostname);
        default:
            ESP_LOGE(TAG,
                    "WiFi mode %d not understood. Please check configuration.",
                    system_configuration_wifi->mode
            );
            return ESP_ERR_INVALID_ARG;
    }
}

esp_err_t wifi_deinitialize(wifi_t* wifi, system_configuration_wifi_t* system_configuration_wifi) {
    if (system_configuration_wifi->mode == WIFI_STATION) {
        ERROR_CHECK(esp_wifi_disconnect());
    } else if (system_configuration_wifi->mode == WIFI_AP) {
        ERROR_CHECK(esp_wifi_deauth_sta(0));
    }
    ERROR_CHECK(esp_wifi_stop());
    ERROR_CHECK(esp_wifi_deinit());

    return ESP_OK;
}

static esp_err_t wifi_init_ap(wifi_t* wifi, system_configuration_wifi_t* system_configuration_wifi, const char* hostname) {
    wifi->netif = esp_netif_create_default_wifi_ap();

    ERROR_CHECK(esp_netif_set_hostname(wifi->netif, hostname))

    ERROR_CHECK(wifi_init(wifi));
    
    ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ERROR_CHECK(wifi_configure_ap(system_configuration_wifi));
    ERROR_CHECK(esp_netif_create_ip6_linklocal(wifi->netif));

    return esp_wifi_start();
}

static esp_err_t wifi_configure_ap(system_configuration_wifi_t* system_configuration_wifi) {
    wifi_config_t wifi_config = {
        .ap = {
            .max_connection = WIFI_MAX_CONNECTION,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };


    memcpy(&wifi_config.ap.ssid, &system_configuration_wifi->ssid, sizeof(wifi_config.ap.ssid));
    memcpy(&wifi_config.ap.password, &system_configuration_wifi->password, sizeof(wifi_config.ap.password));

    if (strlen(system_configuration_wifi->password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

    return ESP_OK;
}

static esp_err_t wifi_init(wifi_t* wifi) {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ERROR_CHECK(esp_wifi_init(&cfg));
    ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, (void*)wifi));

    return ESP_OK;
}


static esp_err_t wifi_init_station(wifi_t* wifi, system_configuration_wifi_t* system_configuration_wifi, const char* hostname) {
    wifi->netif = esp_netif_create_default_wifi_sta();
    
    ERROR_CHECK(esp_netif_set_hostname(wifi->netif, hostname))

    wifi->event_group = xEventGroupCreate();

    ERROR_CHECK(wifi_init(wifi));
    ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, (void*)wifi));
    ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ERROR_CHECK(wifi_configure_station(system_configuration_wifi));
    ERROR_CHECK(esp_wifi_start());
    ERROR_CHECK(wifi_wait_connected(wifi));
    ERROR_CHECK(esp_netif_create_ip6_linklocal(wifi->netif));

    return ESP_OK;
}

static esp_err_t wifi_configure_station(system_configuration_wifi_t* system_configuration_wifi) {
    wifi_config_t wifi_config = { 0 };

    memcpy(&wifi_config.sta.ssid, &system_configuration_wifi->ssid, sizeof(wifi_config.sta.ssid));
    memcpy(&wifi_config.sta.password, &system_configuration_wifi->password, sizeof(wifi_config.sta.password));
    ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    return ESP_OK;
}

static esp_err_t wifi_wait_connected(wifi_t* wifi) {
    EventBits_t bits = xEventGroupWaitBits(wifi->event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler));
    // ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler));

    // vEventGroupDelete(wifi->event_group);

    if (bits & WIFI_CONNECTED_BIT) {
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        return ESP_FAIL;
    }

    return ESP_ERR_INVALID_STATE;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    wifi_t* wifi = (wifi_t*)arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ERROR_DISCARD(esp_wifi_connect());
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Connecting with the AP failed. Retrying %d/%d",
                wifi->retry_count, WIFI_MAXIMUM_RETRY);
        wifi->retry_count++;
        ERROR_DISCARD(esp_wifi_connect());
        if (wifi->retry_count == WIFI_MAXIMUM_RETRY) {
            ESP_LOGE(TAG, "Failed to connect");
            xEventGroupSetBits(wifi->event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG, "Connected to station");
        ERROR_DISCARD(esp_netif_create_ip6_linklocal(wifi->netif));
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        wifi->retry_count = 0;
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got DHCP lease:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi->event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}
