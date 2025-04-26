#pragma once

#include <nuttx/config.h>

#ifdef CONFIG_MCP_PINS

#include "mcp_pins_defs.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
   uint8_t socket_id;
   uint8_t pinno;
   bool is_input;
} mcp_pins_dsc_t;

typedef struct {
   uint8_t periph_type;
   uint8_t identifier;
   mcp_pins_dsc_t pins[4];
} mcp_pins_entry_t;

/* Whoever includes this header gets a copy
   so this data does not need to reside in
   user or kernel dataspace exclusively.
 */
static const mcp_pins_entry_t mcp_pins[MCP_PINS_PERIPH_COUNT] = {
   {MCP_PINS_PERIPH_TYPE_SPI, (2 << 4) | 0, {
      [MCP_PINS_PIN_SPI_CLK ] = {0, 0, .is_input=false},
      [MCP_PINS_PIN_SPI_MISO] = {0, 0, .is_input=true },
      [MCP_PINS_PIN_SPI_MOSI] = {0, 1, .is_input=false},
      [MCP_PINS_PIN_SPI_CS  ] = {1, 0, .is_input=false}
   }},
   {MCP_PINS_PERIPH_TYPE_SPI, (2 << 4) | 1, {
      [MCP_PINS_PIN_SPI_CLK ] = {0, 0, .is_input=false},
      [MCP_PINS_PIN_SPI_MISO] = {0, 0, .is_input=true },
      [MCP_PINS_PIN_SPI_MOSI] = {0, 1, .is_input=false},
      [MCP_PINS_PIN_SPI_CS  ] = {1, 1, .is_input=false}
   }},
};

#endif /* CONFIG_MCP_PINS */
