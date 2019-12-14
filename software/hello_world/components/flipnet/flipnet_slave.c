#include "flipnet.h"
#include "flipnet_private.h"

static const char* TAG = "flipnet_slave";

esp_err_t flipnet_init_slave(flipnet_interface_t* interface,
        flipnet_interface_config_t* interface_config)
{
    interface->rx_queue = xQueueCreate(interface_config->rx_queue_size,
            sizeof(flipnet_frame_t));
    if (interface->rx_queue == NULL) {
        ESP_LOGE(TAG, "xQueueCreate failed");
        return ESP_FAIL;
    }
    interface->cobs_buf = malloc(interface_config->mtu);
    if (interface->cobs_buf == NULL) {
        ESP_LOGE(TAG, "malloc(interface_config->mtu) failed");
        return ESP_ERR_NO_MEM;
    }
    xTaskCreate(flipnet_rx_task, "flipnet_rx_task", 8192, (void*)interface, 12,
            &(interface->rx_task));
    return ESP_OK;
}

void flipnet_rx_task(void* param) {
    ESP_LOGI(TAG, "rx task started");
    flipnet_interface_t* interface = (flipnet_interface_t*)param;
    uart_event_t event;
    
    for (;;) {
        if (!xQueueReceive(interface->uart_event_queue,
                    (void*)&event, (portTickType)portMAX_DELAY)) {
            // TODO implement some kind of timeout after receiving no events
            continue;
        }
        heap_caps_check_integrity_all(true);
        flipnet_rx_handle_uart_event(interface, &event);
        heap_caps_check_integrity_all(true);
    }
}

void flipnet_rx_flush(flipnet_interface_t* interface) {
}

esp_err_t flipnet_rx_handle_uart_event(flipnet_interface_t* interface, uart_event_t* uart_event) {
    switch (uart_event->type) {
        case UART_BUFFER_FULL:
            interface->rx_drop_buffer_full++;
            break;
        case UART_PARITY_ERR:
            interface->rx_drop_parity++;
            goto PHY_ERROR;
        case UART_FRAME_ERR:
            interface->rx_drop_frame_error++;
            goto PHY_ERROR;
        case UART_FIFO_OVF:
            interface->rx_drop_fifo_ovf++;
            goto PHY_ERROR;
        case UART_BREAK:
            interface->rx_break_count++;
            goto PHY_ERROR;
        default:
            break;
    }
    if (uart_event->type != UART_DATA &&
        uart_event->type != UART_PATTERN_DET &&
        uart_event->type != UART_BUFFER_FULL)
    {
        ESP_LOGE(TAG, "unexpected uart event");
        return ESP_FAIL;
    }
    return flipnet_rx_receive(interface);

PHY_ERROR:
    ESP_LOGE(TAG, "phy error");
    interface->rx_drop_count++;
    interface->cobs_state.p = interface->cobs_state.c = 0;
    interface->synchronized = false;
    uart_flush_input(FLIPNET_UART_NUM);
    return ESP_FAIL;
}

esp_err_t flipnet_rx_receive(flipnet_interface_t* interface) {
    size_t available = 0;
    uint8_t buf[1];
    ESP_ERROR_CHECK(uart_get_buffered_data_len(FLIPNET_UART_NUM, &available));
    // feed data bytewise into the COBS decoder
    while (available > 0) {
        int received = uart_read_bytes(FLIPNET_UART_NUM, buf, 1, portMAX_DELAY);
        if (received != 1) {
            ESP_LOGE(TAG, "failed to read from the uart");
            interface->cobs_state.p = interface->cobs_state.c = 0;
            interface->synchronized = false;
            return ESP_FAIL;
        }
        if (!interface->synchronized) {
            if (buf[0] == 0) {
                interface->synchronized = true;
            }
        } else {
            int cobs_decode_result = flipnet_cobs_decode_incremental(
                    &interface->cobs_state,
                    interface->cobs_buf,
                    interface->config->mtu,
                    buf[0]);
            if (cobs_decode_result < -1) {
                ESP_LOGE(TAG, "flipnet_cobs_decode_incremental returned %d.",
                        cobs_decode_result);
                interface->cobs_state.p = interface->cobs_state.c = 0;
                interface->synchronized = false;
                interface->rx_drop_count++;
            } else if (cobs_decode_result >= 0) {
                // reset the COBS state machine
                interface->cobs_state.p = interface->cobs_state.c = 0;
                // parse and handle the received frame
                flipnet_rx_handle_frame(interface, cobs_decode_result);
            }
        }
        ESP_ERROR_CHECK(uart_get_buffered_data_len(FLIPNET_UART_NUM, &available));
    }
    return ESP_OK;
}

esp_err_t flipnet_rx_handle_frame(flipnet_interface_t* interface, size_t received_length) {
    esp_err_t frame_decode_result; 
    flipnet_frame_t* frame = calloc(1, sizeof(flipnet_frame_t));
    if (!frame) {
        ESP_LOGE(TAG, "could not allocate memory for received frame");
        interface->cobs_state.c = interface->cobs_state.p = 0;
        return ESP_ERR_NO_MEM;
    }
    heap_caps_check_integrity_all(true);
    frame_decode_result = flipnet_decode_frame(interface->cobs_buf, received_length, frame,
            interface->config->ignore_checksums);
    heap_caps_check_integrity_all(true);
    ESP_LOGI(TAG, "flipnet_decode_frame returned %d", frame_decode_result);
    if (frame_decode_result != ESP_OK) { goto FAIL; }
    if ((frame->address != interface->config->address) &&
        (frame->address != FLIPNET_BROADCAST_ADDRESS) &&
        !interface->config->promiscuous_mode)
    {
        goto CLEAN_UP;
    }
    if (xQueueSendToBack(interface->rx_queue, frame, 0) == pdTRUE) {
        ESP_LOGI(TAG, "frame received to rxQueue");
        free(frame);
        interface->rx_count++;
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "receive queue is full, dropping frame");
        interface->rx_drop_queue_full++;
        goto FAIL;
    }

CLEAN_UP:
    free(frame->payload);
    frame->payload = NULL;
    free(frame);
    frame = NULL;
    return ESP_OK;

FAIL:
    interface->rx_drop_count++;
    free(frame->payload);
    frame->payload = NULL;
    free(frame);
    frame = NULL;
    return ESP_FAIL;
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

esp_err_t flipnet_decode_frame(uint8_t* buffer, uint16_t size, flipnet_frame_t* frame, bool ignore_checksum) {
    if (frame->payload != NULL) { return ESP_ERR_INVALID_ARG; }
    if (size < 4) { return ESP_ERR_INVALID_SIZE; }
    frame->address = buffer[0];
    frame->command = buffer[1];
    frame->payload_size = size - 4;
    if (frame->payload_size) {
        ESP_LOGI(TAG, "payload size %d", frame->payload_size);
        if ((frame->payload = malloc(frame->payload_size)) == NULL) {
           ESP_LOGE(TAG, "out of memory!"); return ESP_ERR_NO_MEM; }
        memcpy(frame->payload, &buffer[2], frame->payload_size);
    }
    uint16_t received_checksum = buffer[size - 2] << 8 |
        buffer[size - 1];
    uint16_t expect_checksum = flipnet_crc16(buffer, size - 2);
    if (received_checksum != expect_checksum) {
        if (!ignore_checksum) {
            ESP_LOGE(TAG, "expected checksum 0x%.2X 0x%.2X - received checksum 0x%.2X 0x%.2X",
                expect_checksum >> 8, expect_checksum & 0xFF,
                buffer[size - 2], buffer[size - 1]);
            return ESP_ERR_INVALID_CRC;
        } else {
            ESP_LOGI(TAG, "ignoring invalid checksum");
        }
    }
    return ESP_OK;
}
