#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_console.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "main.h"
#include "util.h"
#include "wifi.h"
#include "console_commands.h"
#include "font_rendering.h"

static const char* TAG = "console_commands.c";

static int console_reboot(int argc, char **argv) {
    ESP_LOGI(TAG, "Reboot tud gud!");
    esp_restart();
    return 0;
}

esp_err_t console_register_reboot(void) {
    const esp_console_cmd_t cmd = {
        .command = "reboot",
        .help = "Perform a software reset",
        .hint = NULL,
        .func = &console_reboot,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}

static int console_exit(int argc, char **argv) {
    // TODO anyone knows how to tell `screen` to exit from a serial port?
    putchar(EOF);
    return 0;
}

esp_err_t console_register_exit(void) {
    const esp_console_cmd_t cmd = {
        .command = "exit",
        .help = "Send EOF to exit screen",
        .hint = NULL,
        .func = &console_exit,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}

static int console_show_tasks(int argc, char **argv) {
    /* See vTaskList: "Approximately 40 bytes per task should be sufficient"
       Going for 42, for good luck xD
     */
    const size_t bytes_per_task = 42;
    char *task_list_buffer = malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_list_buffer == NULL) {
        ESP_LOGE(TAG, "failed to allocate buffer for vTaskList output");
        return 1;
    }
    fputs("Task Name\tStatus\tPrio\tHWM\tTask#", stdout);
#ifdef CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
    fputs("\tAffinity", stdout);
#endif
    fputs("\n", stdout);
    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);
    free(task_list_buffer);
    return 0;
}

esp_err_t console_register_show_tasks(void) {
    const esp_console_cmd_t cmd = {
        .command = "show_tasks",
        .help = "Get information about running tasks",
        .hint = NULL,
        .func = &console_show_tasks,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}

static int console_show_version(int argc, char **argv) {
    esp_chip_info_t info;
    esp_chip_info(&info);
    printf("IDF Version:%s\r\n", esp_get_idf_version());
    printf("Chip info:\r\n");
    printf("\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknow");
    printf("\tcores:%d\r\n", info.cores);
    printf("\tfeature:%s%s%s%s%d%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
    printf("\trevision number:%d\r\n", info.revision);
    return 0;
}

esp_err_t console_register_show_version(void) {
    const esp_console_cmd_t cmd = {
        .command = "show_version",
        .help = "Get version of chip and SDK",
        .hint = NULL,
        .func = &console_show_version,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}

static int console_config_save(int argc, char **argv) {
    esp_err_t err = system_configuration_save(&system_configuration);
    if (err != ESP_OK) {
        return 1;
    }
    return 0;
}

esp_err_t console_register_config_save(void) {
    const esp_console_cmd_t cmd = {
        .command = "config_save",
        .help = "Save the current system configuration to flash",
        .hint = NULL,
        .func = &console_config_save,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}

static int console_config_load(int argc, char **argv) {
    esp_err_t err = system_configuration_load(&system_configuration);
    if (err != ESP_OK) {
        return 1;
    }
    return 0;
}

esp_err_t console_register_config_load(void) {
    const esp_console_cmd_t cmd = {
        .command = "config_load",
        .help = "Load the system configuration from flash",
        .hint = NULL,
        .func = &console_config_load,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}

static int console_config_show(int argc, char **argv) {
    system_configuration_dump(&system_configuration);
    return 0;
}

esp_err_t console_register_config_show(void) {
    const esp_console_cmd_t cmd = {
        .command = "config_show",
        .help = "Show the current system configuration",
        .hint = NULL,
        .func = &console_config_show,
    };
    
    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}

static int console_config_reset(int argc, char **argv) {
    system_configuration_load_defaults(&system_configuration);
    return 0;
}

esp_err_t console_register_config_reset(void) {
    const esp_console_cmd_t cmd = {
        .command = "config_reset",
        .help = "Factory reset the system configuration",
        .hint = NULL,
        .func = &console_config_reset,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}

static struct {
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} console_wifi_args;

static int console_config_wifi_ap(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &console_wifi_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, console_wifi_args.end, argv[0]);
        return 1;
    }

    system_configuration.wifi.mode = WIFI_AP;
    strncpy(system_configuration.wifi.ssid,
            console_wifi_args.ssid->sval[0],
            sizeof(system_configuration.wifi.ssid));
    if (console_wifi_args.password->sval[0]) {
        strncpy(system_configuration.wifi.password,
                console_wifi_args.password->sval[0],
                sizeof(system_configuration.wifi.password));
    }

    return 0;
}

esp_err_t console_register_config_wifi_ap(void) {
    console_wifi_args.ssid = arg_str1(NULL, NULL, "<ssid>", "WiFi SSID");
    console_wifi_args.password = arg_str0(NULL, NULL, "<password>", "WiFi Password");
    console_wifi_args.end = arg_end(2);
    
    const esp_console_cmd_t cmd = {
        .command = "config_wifi_ap",
        .help = "Configure AP mode",
        .hint = NULL,
        .func = &console_config_wifi_ap,
        .argtable = &console_wifi_args,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}

static int console_config_wifi_station(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &console_wifi_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, console_wifi_args.end, argv[0]);
        return 1;
    }

    system_configuration.wifi.mode = WIFI_STATION;
    strncpy(system_configuration.wifi.ssid,
            console_wifi_args.ssid->sval[0],
            sizeof(system_configuration.wifi.ssid));
    if (console_wifi_args.password->sval[0]) {
        strncpy(system_configuration.wifi.password,
                console_wifi_args.password->sval[0],
                sizeof(system_configuration.wifi.password));
    }

    return 0;
}

esp_err_t console_register_config_wifi_station(void) {
    console_wifi_args.ssid = arg_str1(NULL, NULL, "<ssid>", "WiFi SSID");
    console_wifi_args.password = arg_str0(NULL, NULL, "<password>", "WiFi Password");
    console_wifi_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "config_wifi_station",
        .help = "Configure station mode",
        .hint = NULL,
        .func = &console_config_wifi_station,
        .argtable = &console_wifi_args,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}

static struct {
    struct arg_str *hostname;
    struct arg_end *end;
} console_hostname_args;

static int console_config_hostname(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &console_hostname_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, console_hostname_args.end, argv[0]);
        return 1;
    }
    strncpy(system_configuration.hostname,
            console_hostname_args.hostname->sval[0],
            sizeof(system_configuration.hostname));

    return 0;
}

esp_err_t console_register_config_hostname(void) {
    console_hostname_args.hostname = arg_str1(NULL, NULL, "<hostname>", "Hostname");
    console_hostname_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "config_hostname",
        .help = "Set system hostname",
        .hint = NULL,
        .func = &console_config_hostname,
        .argtable = &console_hostname_args,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}
