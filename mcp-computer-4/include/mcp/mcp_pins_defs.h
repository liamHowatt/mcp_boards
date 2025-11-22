#pragma once

#include <nuttx/config.h>

#define MCP_PINS_PERIPH_COUNT 3

typedef enum {
    MCP_PINS_PERIPH_TYPE_SPI,
    MCP_PINS_PERIPH_TYPE_UART,
    MCP_PINS_PERIPH_TYPE_LAST_
} mcpd_pins_periph_type_t;

typedef enum {
    MCP_PINS_DRIVER_TYPE_SPI_RAW = 0,
    MCP_PINS_DRIVER_TYPE_SPI_SDCARD,
    MCP_PINS_DRIVER_TYPE_SPI_LAST_,

    MCP_PINS_DRIVER_TYPE_UART_RAW = 0,
    MCP_PINS_DRIVER_TYPE_UART_LAST_,

} mcpd_pins_driver_type_t;

enum {
    MCP_PINS_PIN_SPI_CLK,
    MCP_PINS_PIN_SPI_MISO,
    MCP_PINS_PIN_SPI_MOSI,
    MCP_PINS_PIN_SPI_CS,
    MCP_PINS_PIN_SPI_LAST_
};

enum {
    MCP_PINS_PIN_UART_TX,
    MCP_PINS_PIN_UART_RX,
    MCP_PINS_PIN_UART_LAST_
};
