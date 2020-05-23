/**
  * @file
  * @brief Flipdot GPIO abstraction
  */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
  * @brief GPIO pin for flipdot power on
  * Setting this GPIO to high switches Q5 using Q2, providing +12V on VFlipdot
  */
#define FLIPDOT_GPIO_PWR_ON (21)

/**
  * @brief Not connected
  * Reading from SPI is not required and results in undefined behaviour as GPIO12 is left floating.
  */
#define FLIPDOT_GPIO_SPI_MISO (12)

/**
  * @brief GPIO pin for flipdot driver registers SER
  * @note SPI is used for controlling registers IC5, IC6 and IC7
  */
#define FLIPDOT_GPIO_SPI_MOSI (13)

/**
  * @brief GPIO pin for flipdor driver registers SRCLK
  * @note SPI is used for controlling registers IC5, IC6 and IC7
  */
#define FLIPDOT_GPIO_SPI_CLK (14)

/**
  * @brief GPIO pin for flipdor driver registers RCLK
  */
#define FLIPDOT_GPIO_RCLK_PIN (19)

/**
  * @brief GPIO pin for flipdot driver registers SCRLR
  */
#define FLIPDOT_GPIO_SRCLR_PIN (18)

/**
  * @brief GPIO pin for flipdot drive registers OE
  */
#define FLIPDOT_GPIO_OE_PIN (15)

/**
  * @brief SPI clock rate
  * @note According to datasheet 8 MHz is within guaranteed limit
  */
#define FLIPDOT_SPI_SPEED_HZ (8 * 1000 * 1000)

/**
  * @brief Bit pattern for selecting no panel
  */
#define FLIPDOT_NO_SELECT (0b00011111)

/**
  * @brief Flipdot driver registers pin out
  * Shift registers IC5, IC6 and IC7 are used as port expansion for the ESP32.
  * This data structure memory maps the pins to human readable code
  */
typedef struct {
    /**
      * @brief values of SELECT[0..4]
      * Setting a single bit to zero causes the corresponding panel select (J2 pin 6..10) to be pulled to +12V.
      * Otherwise driven to GND by IC2.
      * @note Only one panel should be selected at a time, to account for the maximum amount of power the flipdot PCB can drive.
      */
    uint8_t select : 5;

    /**
      * @brief clock signal (inverted)
      * Setting this to zero pulls J3 pin 34 to +12V.
      * Otherwise driven to GND by IC2.
      */
    bool clock : 1;

    /**
      * @brief reset signal (inverted)
      * Setting this to zero pulls J3 pin 31 to +12V.
      * Otherwise driven to GND by IC2.
      */
    bool reset : 1;

    /**
      * @brief column clear driver
      * Setting this to one causes low-side driver Q4 to drive common clear (J3 17..20) to GND.
      * The low-side driver is used to clear a whole column.
      * @note: You must never drive a row and clear at once, as this will blow fuse F1.
      */
    bool clear : 1;

    /**
      * @brief column row driver values
      * 16 row high side driver allow for setting individual pixels.
      * @note: You must never drive a row and clear at once, as this will blow fuse F1.
      */
    uint16_t rows : 16;
} __attribute__((packed)) flipdot_io_state_t;
