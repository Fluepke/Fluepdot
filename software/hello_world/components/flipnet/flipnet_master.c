#include "flipnet.h"
#include "flipnet_private.h"

static const char* TAG = "flipnet_master";

esp_err_t flipnet_init_master(flipnet_interface_t* interface,
        flipnet_interface_config_t* interface_config)
{
    interface->tx_queue = xQueueCreate(interface_config->tx_queue_size,
            sizeof(flipnet_frame_t));
    if (interface->tx_queue == NULL) {
        ESP_LOGE(TAG, "xQueueCreate failed");
        return ESP_FAIL;
    }
    xTaskCreate(flipnet_tx_task, "flipnet_tx_task", 8192, (void*)interface, 12,
            &(interface->tx_task));
    return ESP_OK;
}

void flipnet_tx_task(void* param) {
    ESP_LOGI(TAG, "tx task started");
    flipnet_interface_t* interface = (flipnet_interface_t*)param;
    flipnet_frame_t frame;
    uint8_t* buffer; 
    for (;;) {
        if (!xQueueReceive(interface->tx_queue, (void*)&frame, (portTickType)portMAX_DELAY)) {
            continue;
        }
        if (frame.payload_size + 6 > interface->config->mtu) {
            ESP_LOGE(TAG, "dropping packet, that is larger than the MTU");
            interface->tx_drop_count++;
            free(frame.payload);
            frame.payload = NULL;
        }
        buffer = flipnet_encode_frame(&frame);
        if (buffer == NULL) {
            ESP_LOGE(TAG, "flipnet_encode_frame returned a null pointer");
            interface->tx_drop_count++;
            free(frame.payload);
            frame.payload = NULL;
            continue;
        }
        int len = uart_write_bytes(FLIPNET_UART_NUM, (char*)buffer, frame.payload_size + 6);
        ESP_ERROR_CHECK(uart_wait_tx_done(FLIPNET_UART_NUM, 1000));
        ESP_LOGI(TAG, "sent %d bytes", len);
        interface->tx_count++;
        free(frame.payload);
        frame.payload = NULL;
        free(buffer);
        buffer = NULL;
        vTaskDelay(1);
    }
}

uint8_t* flipnet_encode_frame(flipnet_frame_t* frame) {
    uint16_t int_buffer_size = 4 + frame->payload_size;
    uint8_t* int_buffer = malloc(int_buffer_size);
    uint8_t* out_buffer = malloc(int_buffer_size + 2);
    if (int_buffer == NULL) { return NULL; }
    if (out_buffer == NULL) { free(int_buffer); return NULL; }
    int_buffer[0] = frame->address;
    int_buffer[1] = frame->command;
    memcpy(&int_buffer[2], frame->payload, frame->payload_size);
    uint16_t crc = flipnet_crc16(int_buffer, int_buffer_size - 2);
    int_buffer[int_buffer_size - 2] = crc >> 8;
    int_buffer[int_buffer_size - 1] = crc & 0xFF;
    flipnet_cobs_encode(out_buffer, int_buffer, int_buffer_size);
    free(int_buffer);
    return out_buffer;
}

size_t flipnet_cobs_encode(uint8_t* dst, uint8_t* src, size_t srclen) {
    size_t p = 0;
    while (p <= srclen) {

        char val;
        if (p != 0 && src[p-1] != 0) {
            val = src[p-1];

        } else {
            size_t q = p;
            while (q < srclen && src[q] != 0)
                q++;
            val = (char)q-p+1;
        }

        dst[p] = val;
        p++;
    }
    dst[p] = 0;

    return srclen+2;
}
