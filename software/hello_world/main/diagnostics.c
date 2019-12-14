#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_ota_ops.h"

#include "diagnostics.h"
#include "flipnet.h"

void interface_diagnostics_task(void* param) {
    flipnet_interface_t* interface = (flipnet_interface_t*)param;
    for (;;) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI("diagnostics", "interface stats: rx %ld, drop rx %ld\n"
                "buffer fulls: %ld\n"
                "parity errors: %ld\n"
                "frame errors: %ld\n"
                "fifo overflows: %ld\n"
                "breaks: %ld\n"
                "queue fulls: %ld\n",
                interface->rx_count,
                interface->rx_drop_count,
                interface->rx_drop_buffer_full,
                interface->rx_drop_parity,
                interface->rx_drop_frame_error,
                interface->rx_drop_fifo_ovf,
                interface->rx_break_count,
                interface->rx_drop_queue_full);
        ESP_LOGI("diagnostics", "free heap %d", xPortGetFreeHeapSize());
        if (interface->rx_count > 10) {
            ESP_ERROR_CHECK(esp_ota_mark_app_valid_cancel_rollback());
        }
    }
}
