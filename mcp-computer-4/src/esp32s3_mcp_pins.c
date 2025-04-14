#include <nuttx/config.h>

#ifdef CONFIG_MCP_PINS

#include <arch/board/boardctl.h>
#include <arch/board/mcp/mcp_pins_defs.h>
#include <nuttx/mmcsd.h>
#include <debug.h>
#include <assert.h>

#include "esp32s3_spi.h"
#include "esp32s3_board_spidev.h"
#include "esp32s3-devkit.h"

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#if defined(CONFIG_ESP32S3_SPI) && defined(CONFIG_MMCSD_SPI)
static int mmcsd_spi_driver_init(int port, int slot)
{
  int ret = OK;
  struct spi_dev_s *spi;

  syslog(LOG_INFO, "Initializing /dev/mmcsd%d...\n", port);

  /* Initialize SPI device */

  spi = esp32s3_spibus_initialize(port);

  if (spi == NULL)
    {
      syslog(LOG_ERR, "Failed to initialize mmcsd SPI%d.\n", port);
      return -ENODEV;
    }

  ret = mmcsd_spislotinitialize(port, slot, spi);
  if (ret < 0)
    {
      syslog(LOG_ERR, "Failed to register /dev/mmcsd%d: %d\n", port, ret);
      esp32s3_spibus_uninitialize(spi);
    }

  return ret;
}
#endif /* defined(CONFIG_ESP32S3_SPI) && defined(CONFIG_MMCSD_SPI) */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int board_mcp_pins(uintptr_t arg)
{
  DEBUGASSERT(arg);

  struct mcp_pins_s * pins = (struct mcp_pins_s *) arg;
  bool * in_use_p = NULL;
  int ret = OK;

  switch (pins->peripheral)
    {
      case MCP_PINS_PERIPH_TYPE_SPI:
#ifdef CONFIG_ESP32S3_SPI
        {
          int port;

          switch (pins->minor)
            {
              case 2:
#ifdef CONFIG_ESP32S3_SPI2
                {
                  static bool in_use;
                  in_use_p = &in_use;
                  port = ESP32S3_SPI2;
                }
                break;
#else  /* CONFIG_ESP32S3_SPI2 */
                return -ENODEV;
#endif

              case 3:
#ifdef CONFIG_ESP32S3_SPI3
                {
                  static bool in_use;
                  in_use_p = &in_use;
                  port = ESP32S3_SPI3;
                }
                break;
#else  /* CONFIG_ESP32S3_SPI3 */
                return -ENODEV;
#endif

              default:
                return -EINVAL;
            }

          if(*in_use_p)
            {
              return -EBUSY;
            }

          switch (pins->driver)
            {
              case MCP_PINS_DRIVER_TYPE_SPI_RAW:
#ifdef CONFIG_SPI_DRIVER
                ret = board_spidev_initialize(port);
                break;
#else /* CONFIG_SPI_DRIVER */
                return -ENODEV;
#endif

              case MCP_PINS_DRIVER_TYPE_SPI_SDCARD:
#ifdef CONFIG_MMCSD_SPI
                {
                  static uint8_t slot;
                  if(slot < CONFIG_MMCSD_NSLOTS)
                    {
                      ret = mmcsd_spi_driver_init(port, slot++);
                    }
                  else
                    {
                      ret = -ENODEV;
                    }
                }
                break;
#else /* CONFIG_MMCSD_SPI */
                return -ENODEV;
#endif

              default:
                return -EINVAL;
            }
        }
        break;
#else /* CONFIG_ESP32S3_SPI */
        return -ENODEV;
#endif

      default:
        return -EINVAL;
    }

  if(ret == OK && in_use_p != NULL)
    {
      *in_use_p = true;
    }

  return ret;
}

#endif /* CONFIG_MCP_PINS */
