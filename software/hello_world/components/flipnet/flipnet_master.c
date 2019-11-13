#include "flipnet.h"
#include "flipnet_private.h"

static const char* TAG = "flipnet_master";

void flipnet_tx_task(void* param) {
    ESP_LOGI(TAG, "tx task started");
    flipnet_interface_t* interface = (flipnet_interface_t*)param;
    
    // example frame
    flipnet_frame_t frame;
    frame.address = 42;
    frame.command = FRAMEBUFFER;
    frame.payload_size = 230;
    uint8_t example_payload[230];
    uint8_t pattern = 0b10101010;
    frame.payload = example_payload;
    uint8_t* buffer;


    for (;;) {
        pattern = ~pattern;
        memset(example_payload, pattern, 230);
        if ((buffer = flipnet_encode_frame(&frame)) == NULL) {
            ESP_LOGE(TAG, "frame encode failed");
            vTaskDelete(NULL);
            return;
        }
        ESP_LOGI(TAG, "write payload");
        flipnet_printf_buffer(buffer, frame.payload_size + 6);
        if (frame.payload_size + 4 > interface->config->mtu * 2) {
            ESP_LOGE(TAG, "trying to transmit a frame, that is larger than the MTU!!");
        }
        int len = uart_write_bytes(FLIPNET_UART_NUM, (char*)buffer, frame.payload_size + 6);
        ESP_LOGI(TAG, "sent %d bytes", len);
        // wait for transmission to finish
        ESP_LOGI(TAG, "wait for transmission to finish");
        ESP_ERROR_CHECK(uart_wait_tx_done(FLIPNET_UART_NUM, 1000));
        ESP_LOGI(TAG, "remaining FIFO %d", UART2.status.txfifo_cnt);
        ESP_LOGI(TAG, "Transmitted a frame");
        vTaskDelay(10);
        free(buffer);
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
