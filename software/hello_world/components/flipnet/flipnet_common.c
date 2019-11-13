#include "flipnet.h"
#include "flipnet_private.h"

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

void flipnet_printf_buffer(uint8_t* buffer, uint16_t size) {
    for (int i=0; i<size; i++) {
       printf("0x%.2X ", buffer[i]);
    }
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
