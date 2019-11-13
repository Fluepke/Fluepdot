#include "dmx.h"

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

static const char* TAG = "flipnet";

esp_err_t flipnet_init(flipnet_interface_t* interface, flipnet_interface_config_t* interface_config) {
    if (interface_config->mtu > FLIPNET_MAX_MTU) { return ESP_ERR_INVALID_ARG; }
    uart_config_t uart_config = {
        .baud_rate = interface_config->baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    ESP_ERROR_CHECK(uart_param_config(FLIPNET_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(FLIPNET_UART_NUM, FLIPNET_TXD_PIN, FLIPNET_RXD_PIN, FLIPNET_RTS_PIN, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(FLIPNET_UART_NUM,
                interface_config->mtu + 2, // rx_buffer_size
                interface_config->mtu + 2, // tx_buffer_size
                FLIPNET_UART_QUEUE_SIZE, // queue_size
                &(interface->uart_event_queue),
                0));
    ESP_ERROR_CHECK(uart_set_mode(FLIPNET_UART_NUM, UART_MODE_RS485_HALF_DUPLEX));
    if (interface_config->mode == SLAVE) {
        interface->rx_queue = xQueueCreate(FLIPNET_RX_QUEUE_SIZE, sizeof(flipnet_frame_t));
        if (interface->rx_queue == NULL) {
            ESP_LOGE(TAG, "xQueueCreate failed");
            return ESP_FAIL;
        }
        uart_enable_pattern_det_baud_intr(FLIPNET_UART_NUM,
                FLIPNET_START_OF_FRAME,
                FLIPNET_START_OF_FRAME_COUNT,
                1000,0, 0);
        ets_delay_us(100);
        xTaskCreate(flipnet_rx_task, "flipnet_rx_task", 8192, (void*)interface, 12, &(interface->rx_task));
    } else {
        xTaskCreate(flipnet_tx_task, "flipnet_tx_task", 8192, (void*)interface, 12, &(interface->tx_task));
    }
    interface->config = interface_config;
    return ESP_OK;
}

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

void flipnet_printf_buffer(uint8_t* buffer, uint16_t size) {
    for (int i=0; i<size; i++) {
       printf("0x%.2X ", buffer[i]);
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

esp_err_t flipnet_decode_frame(uint8_t* buffer, uint16_t size, flipnet_frame_t* frame) {
    if (frame->payload != NULL) { ESP_LOGE(TAG, "payload pointer was not null"); return ESP_ERR_INVALID_ARG; }
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

uint16_t flipnet_crc16(uint8_t* buffer, uint16_t size) {
    uint16_t out = 0;
    int bits_read = 0, bit_flag;

    /* Sanity check: */
    if(buffer == NULL)
        return 0;

    while(size > 0)
    {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*buffer >> bits_read) & 1; // item a) work from the least significant bits

        /* Increment bit counter: */
        bits_read++;
        if(bits_read > 7)
        {
            bits_read = 0;
            buffer++;
            size--;
        }

        /* Cycle check: */
        if(bit_flag)
            out ^= CRC16;

    }

    // item b) "push out" the last 16 bits
    int i;
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if(bit_flag)
            out ^= CRC16;
    }

    // item c) reverse the bits
    uint16_t crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>=1, j <<= 1) {
        if (i & out) crc |= j;
    }

    return crc;
}
