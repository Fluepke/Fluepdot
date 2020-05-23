#pragma once

#include "esp_err.h"
#include "argtable3/argtable3.h"

esp_err_t console_register_reboot(void);
esp_err_t console_register_show_tasks(void);
esp_err_t console_register_show_version(void);
esp_err_t console_register_config_save(void);
esp_err_t console_register_config_load(void);
esp_err_t console_register_config_show(void);
esp_err_t console_register_config_reset(void);
esp_err_t console_register_config_wifi_ap(void);
esp_err_t console_register_config_wifi_station(void);
esp_err_t console_register_config_hostname(void);
esp_err_t console_register_config_panel_layout(void);
esp_err_t console_register_flipdot_clear(void);
