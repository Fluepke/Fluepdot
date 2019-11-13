#pragma once
#include <stdio.h>

typedef struct {
    size_t p;
    size_t c;
} flipnet_cobs_decode_state_t;

int flipnet_cobs_decode_incremental(flipnet_cobs_decode_state_t* state, uint8_t* dst, size_t dstlen, uint8_t src);
size_t flipnet_cobs_encode(uint8_t *dst, uint8_t *src, size_t srclen);
esp_err_t flipnet_decode_frame(uint8_t* buffer, uint16_t size, flipnet_frame_t* frame);
uint8_t* flipnet_encode_frame(flipnet_frame_t* frame);
uint16_t flipnet_crc16(uint8_t* buffer, uint16_t size);
void flipnet_printf_buffer(uint8_t* buffer, uint16_t size);
void flipnet_rx_task(void* param);
void flipnet_tx_task(void* param);
