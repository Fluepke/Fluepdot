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
// this is a limitation imposed by the COBS implementation in use
#define FLIPNET_MAX_MTU (0xFE)
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
     * Turn the flipdot on or off
     */
    POWER = 1,
    /**
     * Transmit a bitmap framebuffer. Expects 230 bytes payload
     * TODO: Explain how the framebuffer works
     */
    FRAMEBUFFER = 2,
    /**
     * Render the framebuffer
     */
    RENDER = 3,
    /**
     * Tell the board how to render the frame
     */
    // RENDERING_OPTIONS0 = 4,
    // RENDERING_OPTIONS1 = 5,
    /**
     * How long to power the "clear" line for each column
     */
    CLEAR_OPTIONS = 4,
    /**
     * How long to power the "y" channels for each column
     */
    FLUEP_OPTIONS = 5,
    /**
     * Render the text that was provided as payload
     * Remember to null terminate the string
     */
    TEXT = 6,
    /**
     * Request the flipdot to load the given font name
     */
    LOAD_FONT = 7
} flipnet_command_t;

typedef struct {
    size_t p;
    size_t c;
} flipnet_cobs_decode_state_t;

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
    // transmit queue size
    size_t tx_queue_size;
    // receive queue size
    size_t rx_queue_size;
    // if set to true, frames with invalid checksums will be received
    bool ignore_checksums;
} flipnet_interface_config_t;

/**
 * Interface handle
 */
typedef struct {
    // used internally for handling uart events
    QueueHandle_t uart_event_queue;
    // queue that stores received @see flipnet_frame_t frames
    // user must ensure to free() frame->payload
    QueueHandle_t rx_queue;
    // queue thtat stores outgoind @seee flipnet_frame_t frames
    QueueHandle_t tx_queue;
    // receiver task handle
    TaskHandle_t rx_task;
    // sender task handle
    TaskHandle_t tx_task;
    // interface configuration
    flipnet_interface_config_t* config;
    // received flipnet_frame_t frames
    unsigned long rx_count;
    // total receive errors
    unsigned long rx_drop_count;
    // uart buffer full errors
    unsigned long rx_drop_buffer_full;
    // receive parity erros
    unsigned long rx_drop_parity;
    // receive (hw) frame errors
    unsigned long rx_drop_frame_error;
    // receive fifo overflows
    unsigned long rx_drop_fifo_ovf;
    // packet drops due to queue overflow
    unsigned long rx_drop_queue_full;
    // break signals received
    unsigned long rx_break_count;
    // transmit count
    unsigned long tx_count;
    // transmit errors
    unsigned long tx_drop_count;
    // is receiver synchronized with sender?
    bool synchronized;
    // internal state machine for decoding frames
    flipnet_cobs_decode_state_t cobs_state;
    // buffer for COBS decode
    uint8_t* cobs_buf;
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
