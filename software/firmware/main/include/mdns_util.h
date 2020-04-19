#pragma once

#include "esp_err.h"

#define MDNS_INSTANCE_NAME ("Fluepdot - see gitlab.com/fluepke/fluepdot")

esp_err_t mdns_initialize(const char* hostname);
