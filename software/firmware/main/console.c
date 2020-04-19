#include "esp_spi_flash.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

#include "console.h"
#include "console_commands.h"
#include "cmd_ping.h"
#include "cmd_host.h"
#include "cmd_traceroute.h"
#include "cmd_font_rendering.h"
#include "util.h"

static const char* TAG = "console.c";
char prompt[50];

esp_err_t console_initialize(system_configuration_t* system_configuration) {
    ERROR_CHECK(console_initialize_drivers());
    esp_console_register_help_command();
    register_ping();
    ERROR_CHECK(register_host_cmd());
    ERROR_CHECK(register_traceroute_cmd());
    ERROR_CHECK(console_register_reboot());
    ERROR_CHECK(console_register_exit());
    ERROR_CHECK(console_register_show_version());
    ERROR_CHECK(console_register_show_tasks());
    ERROR_CHECK(console_register_config_save());
    ERROR_CHECK(console_register_config_load());
    ERROR_CHECK(console_register_config_show());
    ERROR_CHECK(console_register_config_reset());
    ERROR_CHECK(console_register_config_wifi_ap());
    ERROR_CHECK(console_register_config_wifi_station());
    ERROR_CHECK(console_register_config_hostname());
    ERROR_CHECK(console_register_show_fonts());
    ERROR_CHECK(console_register_render_font());

    snprintf(prompt, sizeof(prompt), LOG_COLOR_I "%s> " LOG_RESET_COLOR, system_configuration->hostname);

    xTaskCreate(console_task, "console", 4096, NULL, 8, NULL);

    return ESP_OK;
}

esp_err_t console_initialize_drivers() {
    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Install UART driver for interrupt-driven reads and writes */
    ERROR_CHECK(uart_driver_install(CONSOLE_UART_NUM, 256, 0, 0, NULL, 0));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONSOLE_UART_NUM);

    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_args = 8,
            .max_cmdline_length = 256,
            .hint_color = atoi(LOG_COLOR_CYAN)
    };
    ERROR_CHECK(esp_console_init(&console_config));

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    linenoiseHistorySetMaxLen(6);

    return ESP_OK;
}

void console_task(void* param) {
    printf("\n"
           "This is the fluepdot shell.\n"
           "Type 'help' to get the list of commands.\n"
           "Use UP/DOWN arrows to navigate through command history.\n"
           "Press TAB when typing command name to auto-complete.\n");

    int probe_status = linenoiseProbe();
    if (probe_status) { /* zero indicates success */
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
    }

    while (true) {
        char* line = linenoise(prompt);
        if (line == NULL) {
            continue;
        }

        linenoiseHistoryAdd(line);
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
        } else if (err != ESP_OK) {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }
}
