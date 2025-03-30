#pragma once

#include "mcp_pins_defs.h"

#include <stdint.h>
#include <stdbool.h>

#define MCP_PINS_COUNT (sizeof(mcp_pins) / sizeof(*mcp_pins))

typedef struct {
   uint8_t socket_id;
   uint8_t pinno;
   bool is_input;
} mcp_pins_dsc_t;

typedef struct {
   const char * path;
   uint8_t type;
   mcp_pins_dsc_t pins[4];
} mcp_pins_entry_t;

/* Whoever includes this header gets a copy
   so this data does not need to reside in
   user or kernel dataspace exclusively.
 */
static const mcp_pins_entry_t mcp_pins[] = {
   {"/dev/spi2", MCP_PINS_TYPE_SPI, {
      [MCP_PINS_SPI_CLK ] = {0, 0, .is_input=false},
      [MCP_PINS_SPI_MISO] = {0, 0, .is_input=true },
      [MCP_PINS_SPI_MOSI] = {0, 1, .is_input=false},
      [MCP_PINS_SPI_CS  ] = {0, 3, .is_input=false}
   }}
};
