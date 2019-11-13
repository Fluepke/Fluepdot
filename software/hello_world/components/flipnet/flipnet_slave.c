#include "flipnet.h"
#include "flipnet_private.h"

static const char* TAG = "flipnet_slave";

void flipnet_rx_task(void* param) {
    bool synchronized = false;
    flipnet_frame_t* frame;
    flipnet_interface_t* interface = (flipnet_interface_t*)param;
    ESP_LOGI(TAG, "rx task started");
    // stores cobs decoded data
    flipnet_cobs_decode_state_t cobs_state = { 0 };
    uint8_t* cobs_buf = malloc(interface->config->mtu);
    if (cobs_buf == NULL) {
        ESP_LOGE(TAG, "could not allocate rx buf");
        vTaskDelete(NULL);
        return;
    }
    uint8_t* hw_rx_buf = malloc(interface->config->mtu + 2);
    if (hw_rx_buf == NULL) {
        ESP_LOGE(TAG, "could not allocate hw rx buf");
        vTaskDelete(NULL);
        return;
    }
    uart_event_t event;
    
    for (;;) {
        if (!xQueueReceive(interface->uart_event_queue, (void*)&event, (portTickType)portMAX_DELAY)) {
            // TODO implement some kind of timeout after receiving no events
            continue;
        }
        if (event.type == UART_BUFFER_FULL ||
                event.type == UART_PARITY_ERR ||
                event.type == UART_FRAME_ERR ||
                event.type == UART_FIFO_OVF ||
                event.type == UART_BREAK) {
            ESP_LOGE(TAG, "phy detected an error!");
            interface->rx_drop_count++;
            synchronized = false;
            cobs_state.c = cobs_state.p = 0;
            uart_flush_input(FLIPNET_UART_NUM);
            continue;
        }
        if (event.type == UART_DATA && !synchronized) {
            ESP_LOGE(TAG, "received unsynchronized data!");
            synchronized = false;
            interface->rx_drop_count++;
            cobs_state.c = cobs_state.p = 0;
            uart_flush_input(FLIPNET_UART_NUM);
            continue;
        }
        if (event.type == UART_PATTERN_DET) {
            if (!synchronized) {
                ESP_LOGI(TAG, "synchronized");
                synchronized = true;
                uart_flush_input(FLIPNET_UART_NUM);
                cobs_state.c = cobs_state.p = 0;
                continue;
            }
        }
        if (event.type != UART_DATA && event.type != UART_PATTERN_DET) {
            continue;
        }
        // receive data from the uart
        size_t available = 0;
        ESP_ERROR_CHECK(uart_get_buffered_data_len(FLIPNET_UART_NUM, &available));
        int received = uart_read_bytes(FLIPNET_UART_NUM, hw_rx_buf, available, portMAX_DELAY);
        
        // run the COBS decoder
        for (int i=0; i<received; i++) {
            int cobs_decode_result = flipnet_cobs_decode_incremental(
                &cobs_state,
                cobs_buf,
                interface->config->mtu,
                hw_rx_buf[i]);
            if (cobs_decode_result == -1) {
                continue;
            } else if (cobs_decode_result < -1) {
                interface->rx_drop_count++;
                uart_flush_input(FLIPNET_UART_NUM);
                synchronized = false;
                cobs_state.c = cobs_state.p = 0;
                break;
            } else {
                ESP_LOGE(TAG, "received a frame");
                frame = calloc(1, sizeof(flipnet_frame_t));
                esp_err_t frame_decode_result; 
                if (!frame) {
                    ESP_LOGE(TAG, "could not allocate memory for received frame");
                    cobs_state.c = cobs_state.p = 0;
                }
                frame_decode_result = flipnet_decode_frame(cobs_buf, cobs_decode_result, frame);
                ESP_LOGI(TAG, "flipnet_decode_frame returned %d", frame_decode_result);
                if (frame_decode_result != ESP_OK) {
                    free(frame->payload);
                    frame->payload = NULL;
                    free(frame);
                    frame = NULL;
                    break;
                }
                if ((frame->address != interface->config->address) &&
                        (frame->address != FLIPNET_BROADCAST_ADDRESS) &&
                        !interface->config->promiscuous_mode) {
                    break;
                }
                if (xQueueSendToBack(interface->rx_queue, frame, 0) == pdTRUE) {
                    interface->rx_count++;
                } else {
                    ESP_LOGE(TAG, "receive queue is full, dropping frame");
                    interface->rx_drop_count++;
                    free(frame->payload);
                    frame->payload = NULL;
                    free(frame);
                    frame = NULL;
                }
                free(frame);
                frame = NULL;
                cobs_state.c = cobs_state.p = 0;
            }
        }
    }
}

int flipnet_cobs_decode_incremental(flipnet_cobs_decode_state_t* state, uint8_t* dst, size_t dstlen, uint8_t src) {
    if (state->p == 0) {
        if (src == 0) {
            ESP_LOGE(TAG, "invalid framing. An empty frame would be [...] 00 01 00, not [...] 00 00");
            return -3;
        }
        state->c = src;
        state->p++;
        return -1;
    }

    if (!src) {
        if (state->c != 1) {
            ESP_LOGE(TAG, "invalid framing. The skip counter does not hit the end of the frame.");
            return -2;
        }
        size_t rv = state->p-1;
        return rv;
    }

    unsigned char val;
    state->c--;

    if (state->c == 0) {
        state->c = src;
        val = 0;
    } else {
        val = src;
    }

    size_t pos = state->p-1;
    if (pos >= dstlen) {
        ESP_LOGE(TAG, "output buffer too small");
        return -4;
    }
    dst[pos] = val;
    state->p++;
    return -1;
}

esp_err_t flipnet_decode_frame(uint8_t* buffer, uint16_t size, flipnet_frame_t* frame) {
    if (frame->payload != NULL) { return ESP_ERR_INVALID_ARG; }
    if (size < 4) { return ESP_ERR_INVALID_SIZE; }
    frame->address = buffer[0];
    frame->command = buffer[1];
    frame->payload_size = size - 4;
    if ((frame->payload = malloc(frame->payload_size)) == NULL) { return ESP_ERR_NO_MEM; }
    memcpy(frame->payload, &buffer[2], frame->payload_size);
    uint16_t received_checksum = buffer[size - 2] << 8 |
        buffer[size - 1];
    uint16_t expect_checksum = flipnet_crc16(buffer, size - 2);
    if (received_checksum != expect_checksum) {
        ESP_LOGE(TAG, "expected checksum 0x%.2X 0x%.2X - received checksum 0x%.2X 0x%.2X",
                expect_checksum >> 8, expect_checksum & 0xFF,
                buffer[size - 2], buffer[size - 1]);
        return ESP_ERR_INVALID_CRC;
    }
    return ESP_OK;
}
