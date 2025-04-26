#ifndef __BOARDS_XTENSA_ESP32S3_ESP32S3_DEVKIT_INCLUDE_BOARDCTL_H
#define __BOARDS_XTENSA_ESP32S3_ESP32S3_DEVKIT_INCLUDE_BOARDCTL_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <sys/boardctl.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifdef CONFIG_MCP_PINS
#define BIOC_MCP_PINS   (BOARDIOC_USER + 1)
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct mcp_pins_s
{
  uint8_t peripheral;
  uint8_t driver;
  uint8_t identifier;
  uint8_t user_devid_hint;

  uint8_t minor_output;
};

#endif /* __BOARDS_XTENSA_ESP32S3_ESP32S3_DEVKIT_INCLUDE_BOARDCTL_H */
