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
    size_t ssid_len = strnlen(console_wifi_args.ssid->sval[0], sizeof(system_configuration.wifi.ssid));
    bzero(system_configuration.wifi.ssid, sizeof(system_configuration.wifi.ssid));
    strncpy(system_configuration.wifi.ssid,
            console_wifi_args.ssid->sval[0],
            ssid_len);
    bzero(system_configuration.wifi.password, sizeof(system_configuration.wifi.password));
    if (console_wifi_args.password->sval[0]) {
        size_t password_len = strnlen(console_wifi_args.password->sval[0], sizeof(system_configuration.wifi.password));
        strncpy(system_configuration.wifi.password,
                console_wifi_args.password->sval[0],
                password_len);
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
    size_t ssid_len = strnlen(console_wifi_args.ssid->sval[0], sizeof(system_configuration.wifi.ssid));
    bzero(system_configuration.wifi.ssid, sizeof(system_configuration.wifi.ssid));
    strncpy(system_configuration.wifi.ssid,
            console_wifi_args.ssid->sval[0],
            ssid_len);
    bzero(system_configuration.wifi.password, sizeof(system_configuration.wifi.password));
    if (console_wifi_args.password->sval[0]) {
        size_t password_len = strnlen(console_wifi_args.password->sval[0], sizeof(system_configuration.wifi.password));
        strncpy(system_configuration.wifi.password,
                console_wifi_args.password->sval[0],
                password_len);
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
    size_t len = strnlen(console_hostname_args.hostname->sval[0], sizeof(system_configuration.hostname));
    bzero(system_configuration.hostname, sizeof(system_configuration.hostname));
    strncpy(system_configuration.hostname,
            console_hostname_args.hostname->sval[0],
            len);

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

static struct {
    struct arg_int *panel_sizes;
    struct arg_end *end;
} console_config_panel_layout_args;

static int console_config_panel_layout(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &console_config_panel_layout_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, console_config_panel_layout_args.end, argv[0]);
    }
    
    int panel_count = console_config_panel_layout_args.panel_sizes->count;

    if (panel_count <= 0 || panel_count >= FLIPDOT_MAX_SUPPORTED_PANELS) {
        printf("Panel count of %d not allowed", panel_count);
        return -1;
    }
    system_configuration.flipdot.panel_count = panel_count;
    for (int i=0; i<FLIPDOT_MAX_SUPPORTED_PANELS; i++) {
        if (i < panel_count) {
            system_configuration.flipdot.panel_size[i] = console_config_panel_layout_args.panel_sizes->ival[i];
        } else {
            system_configuration.flipdot.panel_size[i] = 0;
        }
    }
    return 0;
}

esp_err_t console_register_config_panel_layout(void) {
    console_config_panel_layout_args.panel_sizes = arg_intn(NULL, NULL, "<panel_size>", 1, FLIPDOT_MAX_SUPPORTED_PANELS, "Panel size");
    console_config_panel_layout_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "config_panel_layout",
        .help = "Configure panel count and sizes",
        .hint = NULL,
        .func = &console_config_panel_layout,
        .argtable = &console_config_panel_layout_args,
    };

    return esp_console_cmd_register(&cmd);
}

static struct {
    struct arg_lit* invert;
    struct arg_end* end;
} console_flipdot_clear_args;


static int console_flipdot_clear(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &console_flipdot_clear_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, console_flipdot_clear_args.end, argv[0]);
        return -1;
    }

    uint8_t pattern = 0;
    if (console_flipdot_clear_args.invert->count > 0) {
        pattern = 0xFF;
    }
    memset(flipdot.framebuffer->columns, pattern, 2 * flipdot.width);
    flipdot_set_dirty_flag(&flipdot);

    return 0;
}

esp_err_t console_register_flipdot_clear(void) {
    console_flipdot_clear_args.invert = arg_litn(NULL, "invert", 0, 1, "Set all pixels to white instead of black");
    console_flipdot_clear_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "flipdot_clear",
        .help = "Clear the flipdot",
        .hint = NULL,
        .func = &console_flipdot_clear,
        .argtable = &console_flipdot_clear_args,
    };

    return esp_console_cmd_register(&cmd);
}
