#pragma once

#include "esp_err.h"

#include "system_configuration.h"

#define CONSOLE_UART_NUM (0)

esp_err_t console_initialize(system_configuration_t* system_configuration);
esp_err_t console_initialize_drivers();

void console_task(void* param);
