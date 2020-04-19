#include "mdns_util.h"

#include "mdns.h"

#include "system_configuration.h"
#include "util.h"

static const char* TAG = "mdns.c";

esp_err_t mdns_initialize(const char* hostname)
{
    ERROR_CHECK(mdns_init());
    ERROR_CHECK(mdns_hostname_set(hostname));

    ERROR_CHECK(mdns_instance_name_set(MDNS_INSTANCE_NAME));

    ERROR_CHECK(mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0));

    return ESP_OK;
}
