#include "flipnet.h"
#include "flipnet_private.h"

static const char* TAG = "flipnet";

esp_err_t flipnet_init(flipnet_interface_t* interface,
        flipnet_interface_config_t* interface_config)
{
    if (interface_config->mtu > FLIPNET_MAX_MTU) { return ESP_ERR_INVALID_ARG; }
    if ((interface->rx_queue != NULL) || (interface->tx_queue != NULL)) {
        ESP_LOGE(TAG, "cannot reconfigure an already initialized interface. sorry.");
        return ESP_ERR_INVALID_STATE;
    }
    flipnet_reset_counters(interface);
    flipnet_init_uart(interface, interface_config);

    esp_err_t ret = (interface_config->mode == SLAVE) ?
        flipnet_init_slave(interface, interface_config) :
        flipnet_init_master(interface, interface_config);
    interface->config = interface_config;
    return ret;
}

esp_err_t flipnet_init_uart(flipnet_interface_t* interface,
        flipnet_interface_config_t* interface_config)
{
    uart_config_t uart_config = {
        .baud_rate = interface_config->baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_2,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    size_t hw_rx_buf_size = (interface_config->mode == SLAVE) ?
        2 * (interface_config->baudrate / (1000 / portTICK_PERIOD_MS)) :
        interface_config->mtu + 2;
    ESP_ERROR_CHECK(uart_param_config(FLIPNET_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(FLIPNET_UART_NUM, FLIPNET_TXD_PIN, FLIPNET_RXD_PIN, FLIPNET_RTS_PIN, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(FLIPNET_UART_NUM,
                hw_rx_buf_size,
                interface_config->mtu + 2,
                FLIPNET_UART_QUEUE_SIZE,
                &(interface->uart_event_queue),
                0));
    ESP_ERROR_CHECK(uart_set_mode(FLIPNET_UART_NUM, UART_MODE_RS485_HALF_DUPLEX));
    return ESP_OK;
}

void flipnet_reset_counters(flipnet_interface_t* interface) {
    interface->rx_count = 0;
    interface->rx_drop_count = 0;
    interface->rx_drop_buffer_full = 0;
    interface->rx_drop_parity = 0;
    interface->rx_drop_frame_error = 0;
    interface->rx_drop_fifo_ovf = 0;
    interface->rx_drop_queue_full = 0;
    interface->tx_count = 0;
    interface->tx_drop_count = 0;
}

void flipnet_printf_buffer(uint8_t* buffer, uint16_t size) {
    for (int i=0; i<size; i++) {
       printf("0x%.2X ", buffer[i]);
    }
}

// python equivalent of this function:
// crcmod.mkCrcFun(0x18005, rev=True, initCrc=0x0, xorOut=0x0)
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
