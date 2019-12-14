#pragma once
#include <stdio.h>

int flipnet_cobs_decode_incremental(flipnet_cobs_decode_state_t* state, uint8_t* dst, size_t dstlen, uint8_t src);
size_t flipnet_cobs_encode(uint8_t *dst, uint8_t *src, size_t srclen);
// derserializes a frame from a received buffer and performs checksum validations
esp_err_t flipnet_decode_frame(uint8_t* buffer, uint16_t size, flipnet_frame_t* frame, bool ignore_checksum);
uint8_t* flipnet_encode_frame(flipnet_frame_t* frame);
uint16_t flipnet_crc16(uint8_t* buffer, uint16_t size);
void flipnet_printf_buffer(uint8_t* buffer, uint16_t size);
void flipnet_rx_task(void* param);
void flipnet_tx_task(void* param);

// configures the hw uart for slave as well as master operations
esp_err_t flipnet_init_uart(flipnet_interface_t* interface, flipnet_interface_config_t* interface_config);

// configures the interface in the slave mode and starts rx tasks
esp_err_t flipnet_init_slave(flipnet_interface_t* interface, flipnet_interface_config_t* interface_config);

// configures the interface in the master mode and starts tx tasks
esp_err_t flipnet_init_master(flipnet_interface_t* interface, flipnet_interface_config_t* interface_config);

// resets interface statistics
void flipnet_reset_counters(flipnet_interface_t* interface);

// flushes the UART rx buffer and resets the COBS state machine
void flipnet_rx_flush(flipnet_interface_t* interface);

// handles UART events
esp_err_t flipnet_rx_handle_uart_event(flipnet_interface_t* interface, uart_event_t* event);

// perfoms receival of data from the uart
esp_err_t flipnet_rx_receive(flipnet_interface_t* interface);

// performs processing of frames once COBS decoder emitted a frame
esp_err_t flipnet_rx_handle_frame(flipnet_interface_t* interface, size_t received_length);
