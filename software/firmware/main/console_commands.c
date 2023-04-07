#include "argtable3/argtable3.h"
#include "esp_err.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include <stdint.h>
#include <string.h>
#include "flipdot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "main.h"
#include "util.h"
#include "base64.h"
#include "wifi.h"
#include "console_commands.h"
#include "font_rendering.h"
#include "esp_netif.h"

static const char* TAG = "console_commands.c";

static const char *s_ipv6_addr_types[] = {
    "ESP_IP6_ADDR_IS_UNKNOWN",
    "ESP_IP6_ADDR_IS_GLOBAL",
    "ESP_IP6_ADDR_IS_LINK_LOCAL",
    "ESP_IP6_ADDR_IS_SITE_LOCAL",
    "ESP_IP6_ADDR_IS_UNIQUE_LOCAL",
    "ESP_IP6_ADDR_IS_IPV4_MAPPED_IPV6"
    };

static int console_show_ip(int argc, char **argv) {
    esp_netif_ip_info_t ip;
    ESP_ERROR_CHECK(esp_netif_get_ip_info(wifi.netif, &ip));
    printf("- IPv4 address: " IPSTR "\n", IP2STR(&ip.ip));
    esp_ip6_addr_t ip6[CONFIG_LWIP_IPV6_NUM_ADDRESSES];
    int ip6_addrs = esp_netif_get_all_ip6(wifi.netif, ip6);
    for (int j=0; j< ip6_addrs; ++j) {
        esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&(ip6[j]));
        printf( "- IPv6 address: " IPV6STR ", type: %s\n", IPV62STR(ip6[j]), s_ipv6_addr_types[ipv6_type]);
    }
    return 0;
}

esp_err_t console_register_show_ip(void) {
    const esp_console_cmd_t cmd = {
        .command = "show_ip",
        .help = "Show IP configuration",
        .hint = NULL,
        .func = &console_show_ip,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}

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
    uint32_t size_flash_chip;
    esp_flash_get_size(NULL, &size_flash_chip);
    printf("\tfeature:%s%s%s%s%lu%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           size_flash_chip / (1024 * 1024), " MB");
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
    struct arg_str *rendering_mode;
    struct arg_end *end;
} console_rendering_mode_args;

static int console_config_rendering_mode(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **)&console_rendering_mode_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, console_wifi_args.end, argv[0]);
        return 1;
    }

    if (strcmp(console_rendering_mode_args.rendering_mode->sval[0], "full") == 0) {
        system_configuration.flipdot.rendering_mode = 0;
    } else if (strcmp(console_rendering_mode_args.rendering_mode->sval[0], "differential") == 0) {
        system_configuration.flipdot.rendering_mode = 1;
    } else {
        printf("Must specify 'full' or 'differential'.");
        return 1;
    }

    flipdot.rendering_options->mode = system_configuration.flipdot.rendering_mode;
    return 0;
}

esp_err_t console_register_config_rendering_mode(void) {
    console_rendering_mode_args.rendering_mode = arg_str1(NULL, NULL, "<mode>", "full/differential");
    console_rendering_mode_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "config_rendering_mode",
        .help = "Set default rendering mode to use on power up",
        .hint = NULL,
        .func = &console_config_rendering_mode,
        .argtable = &console_rendering_mode_args,
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

static struct {
    struct arg_str* framebuf64;
    struct arg_end* end;
} console_framebuf64_args;

esp_err_t console_framebuf64(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &console_framebuf64_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, console_framebuf64_args.end, argv[0]);
        return -1;
    }
    
    size_t out_len = 0;
    unsigned char * buf = base64_decode(console_framebuf64_args.framebuf64->sval[0],
        strlen(console_framebuf64_args.framebuf64->sval[0]),
        &out_len);
    if (out_len != 230) {
        printf("Expected that base64 to decode to 230 bytes, got %zu!", out_len);
        return 1;
    }
    if (buf == NULL) {
        printf("base64decode failed!");
        return 2;
    }
    memcpy(flipdot.framebuffer->columns, buf, 230);
    free(buf);
    flipdot_set_dirty_flag(&flipdot);
    return 0;
}

esp_err_t console_register_framebuf64(void) {
    console_framebuf64_args.framebuf64 = arg_str1(NULL, NULL, "<framebuf64>", "base64 encoded 230 bytes long framebuffer");
    console_framebuf64_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "framebuf64",
        .help = "Print a base64 encoded framebuffer",
        .hint = NULL,
        .func = &console_framebuf64,
        .argtable = &console_framebuf64_args,
    };

    ERROR_CHECK(esp_console_cmd_register(&cmd));

    return ESP_OK;
}