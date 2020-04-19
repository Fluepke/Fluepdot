#pragma once

#include "nimble/ble.h"

#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A24
#define BLUETOOTH_FLIPDOT_SERVICE_UUID          0x2342
#define BLUETOOTH_FLIPDOT_LINE0_UUID            0x0000
#define BLUETOOTH_FLIPDOT_LINE1_UUID            0x0001
#define BLUETOOTH_FLIPDOT_LINE2_UUID            0x0002
#define BLUETOOTH_FLIPDOT_LINE3_UUID            0x0003
#define BLUETOOTH_FLIPDOT_LINE4_UUID            0x0004
#define BLUETOOTH_FLIPDOT_LINE5_UUID            0x0005
#define BLUETOOTH_FLIPDOT_LINE6_UUID            0x0006
#define BLUETOOTH_FLIPDOT_LINE7_UUID            0x0007
#define BLUETOOTH_FLIPDOT_LINE8_UUID            0x0008
#define BLUETOOTH_FLIPDOT_LINE9_UUID            0x0009
#define BLUETOOTH_FLIPDOT_LINE10_UUID           0x000A
#define BLUETOOTH_FLIPDOT_LINE11_UUID           0x000B
#define BLUETOOTH_FLIPDOT_LINE12_UUID           0x000C
#define BLUETOOTH_FLIPDOT_LINE13_UUID           0x000D
#define BLUETOOTH_FLIPDOT_LINE14_UUID           0x000E
#define BLUETOOTH_FLIPDOT_LINE15_UUID           0x000F
#define BLUETOOTH_FLIPDOT_WIDTH_UUID            0x0010

struct ble_hs_cfg;

esp_err_t bluetooth_initialize(void);
