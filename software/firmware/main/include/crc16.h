#pragma once

#include <stddef.h>
#include <stdint.h>

#define CRC16 0x8005

uint16_t crc16(const uint8_t *buf, size_t buf_len);
