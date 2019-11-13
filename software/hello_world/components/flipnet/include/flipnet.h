/**
 * Flipnet protocol
 * ================
 *
 * Very simple unidirectional protocol using RS485 for building
 * flipdot installations consisting of multiple panels.
 *
 * Frame layout
 * |    0    |    1    |      2..n      |   n+1   |   n+2   |  n+3 |
 * |:-------:|:-------:|:--------------:|:-------:|:-------:|:----:|
 * | address | command | ... payload .. | CRC16_h | CRC16_l | 0x00 |
 *
 * [COBS](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing) is
 * used for encoding of all transmitted data.
 * The null byte terminator is used for synchronization of sender and
 * receiver.
 *
 * Addressing is done using hardcoded octets in the slaves.
 * A broadcast address exists. Promiscuous mode can be enabled
 * for debugging purposes.
 *
 * Several commands exists, see @see flipnet_command_t.
 *
 * Interaction with the interface takes places using FreeRTOS queues.
 * interface->rx_queue stores received @see flipnet_frame_t frames.
 * interface->tx_queue accepts @see flipnet_frame_t frames.
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_err.h"

#define FLIPNET_UART_NUM UART_NUM_2
#define FLIPNET_RXD_PIN (22)
#define FLIPNET_TXD_PIN (23)
#define FLIPNET_RTS_PIN (25)
#define FLIPNET_UART_QUEUE_SIZE (20)
#define FLIPNET_RX_QUEUE_SIZE (3)
// this is a limitation imposed by the COBS implementation in use
#define FLIPNET_MAX_MTU (0xFE)
#define FLIPNET_ESCAPE_CHAR (0x23)
#define FLIPNET_START_OF_FRAME (0x0)
#define FLIPNET_START_OF_FRAME_COUNT (1)
#define FLIPNET_BROADCAST_ADDRESS (0xFF)
#define CRC16 0x8005

/**
 * Single master, multi slave protocol
 */
typedef enum { MASTER, SLAVE } flipnet_mode_t;
typedef enum {
    /**
     * Clears the entire display
     * Supported payloads:
     * * none: Clears the display with default options
     * * 1 byte: Clear the display with the given duration per channel
     */
    CLEAR = 0,
    /**
     * Transmit a bitmap framebuffer. Expects 230 bytes payload
     * TODO: Explain how the framebuffer works
     */
    FRAMEBUFFER = 1,
    /**
     * Set rendering options
     */
    RENDERING_OPTIONS = 2,
    /**
     * Render the contents of the framebuffer
     */
    RENDER = 3
} flipnet_command_t;

/**
 * Interface configuration
 */
typedef struct {
    // mtu refers to the whole frame, including headers and payload
    const uint16_t mtu;
    // slave's address
    uint8_t address;
    // operational mode of the interface, currently only
    // unidirectional communications are supported
    const flipnet_mode_t mode;
    // baudrate to use, TODO: test which datarate is feasible with lots of noise
    const unsigned long baudrate;
    // the hardware uart to use
    uart_port_t uart_port;
    // enable / disable promiscuous mode
    bool promiscuous_mode;
} flipnet_interface_config_t;

/**
 * Interface handle
 */
typedef struct {
    // used internally for handling uart events
    QueueHandle_t uart_event_queue;
    // queue that stores received @see flipnet_frame_t frames
    QueueHandle_t rx_queue;
    // receiver task handle
    TaskHandle_t rx_task;
    // sender task handle
    TaskHandle_t tx_task;
    // interface configuration
    flipnet_interface_config_t* config;
    // received flipnet_frame_t frames
    unsigned long rx_count;
    // receive errors
    unsigned long rx_drop_count;
} flipnet_interface_t;

/**
 * Frames received / transmitted
 */
typedef struct {
    uint8_t address;
    flipnet_command_t command;
    uint16_t payload_size;
    uint8_t* payload;
} flipnet_frame_t;

/**
 * Initialize flipnet interface with the specified configuration
 * Sets up the hw uart, event queues and required tasks.
 * @brief ifup the rs485 interface
 * @param interface interface handle
 * @param interface_config config to apply
 */
esp_err_t flipnet_init(flipnet_interface_t* interface, flipnet_interface_config_t* interface_config);
