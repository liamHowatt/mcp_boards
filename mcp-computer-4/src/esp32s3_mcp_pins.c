#include <nuttx/config.h>

#ifdef CONFIG_MCP_PINS

#include <arch/board/boardctl.h>
#include <arch/board/mcp/mcp_pins_defs.h>
#include <nuttx/spi/spi_transfer.h>
#include <nuttx/mmcsd.h>
#include <debug.h>
#include <assert.h>

#include "esp32s3_spi.h"
#include "esp32s3_board_spidev.h"
#include "esp32s3_gpio.h"
#include "esp32s3-devkit.h"

#include "hardware/esp32s3_gpio_sigmap.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const uint8_t g_cs_pins[2][2] = {{7, 6}};

static uint8_t g_spi_driver_type[2][2] = {{0xff, 0xff}, {0xff, 0xff}};
static uint8_t g_spi_devid_hints[2][2] = {{0xff, 0xff}, {0xff, 0xff}};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#if defined(CONFIG_ESP32S3_SPI_UDCS) && (defined(CONFIG_ESP32S3_SPI2) || defined(CONFIG_ESP32S3_SPI3))
static void spi_select(int idx, uint32_t devid, bool selected)
{
  int cs_idx = 0;
  if (   g_spi_driver_type[idx][0] == MCP_PINS_DRIVER_TYPE_SPI_RAW
      && g_spi_driver_type[idx][1] == MCP_PINS_DRIVER_TYPE_SPI_RAW)
    {
      if (SPIDEVID_INDEX(devid) == g_spi_devid_hints[idx][1])
        {
          cs_idx = 1;
        }
    }
  else if (g_spi_driver_type[idx][0] == MCP_PINS_DRIVER_TYPE_SPI_SDCARD)
    {
      if (SPIDEVID_TYPE(devid) != SPIDEVTYPE_MMCSD)
        {
          cs_idx = 1;
        }
    }
  else if (g_spi_driver_type[idx][1] == MCP_PINS_DRIVER_TYPE_SPI_SDCARD)
    {
      if (SPIDEVID_TYPE(devid) == SPIDEVTYPE_MMCSD)
        {
          cs_idx = 1;
        }
    }

  esp32s3_gpiowrite(g_cs_pins[idx][cs_idx], !selected);
}
#endif /* defined(CONFIG_ESP32S3_SPI_UDCS) && (defined(CONFIG_ESP32S3_SPI2) || defined(CONFIG_ESP32S3_SPI3)) */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#ifdef CONFIG_ESP32S3_SPI_UDCS
#ifdef CONFIG_ESP32S3_SPI2
void esp32s3_spi2_select(struct spi_dev_s *dev, uint32_t devid, bool selected)
{
  spi_select(0, devid, selected);
}
#endif /* CONFIG_ESP32S3_SPI2 */

#ifdef CONFIG_ESP32S3_SPI3
void esp32s3_spi3_select(struct spi_dev_s *dev, uint32_t devid, bool selected)
{
  spi_select(1, devid, selected);
}
#endif /* CONFIG_ESP32S3_SPI3 */
#endif /* CONFIG_ESP32S3_SPI_UDCS */

int board_mcp_pins(uintptr_t arg)
{
  DEBUGASSERT(arg);

  struct mcp_pins_s * pins = (struct mcp_pins_s *) arg;
  bool * in_use_p = NULL;
  int ret = OK;

  if(pins->user_devid_hint >= 255)
    {
      return -EINVAL;
    }

  switch (pins->peripheral)
    {
      case MCP_PINS_PERIPH_TYPE_SPI:
#ifdef CONFIG_ESP32S3_SPI
        {
          uint8_t port_number = pins->identifier >> 4;
          uint8_t cs_number = pins->identifier & 0x0f;
          struct spi_dev_s ** spi_p;
          uint8_t port_idx = port_number - 2;
          int port;

          if(cs_number >= 2)
            {
              return -EINVAL;
            }

          switch (port_number)
            {
              case 2:
#ifdef CONFIG_ESP32S3_SPI2
                {
                  static struct spi_dev_s * spi;
                  static bool in_use[2];
                  spi_p = &spi;
                  in_use_p = &in_use[cs_number];
                  port = ESP32S3_SPI2;
                  pins->minor_output = 2 + 2 * cs_number;
                }
                break;
#else  /* CONFIG_ESP32S3_SPI2 */
                return -ENODEV;
#endif

              case 3:
#ifdef CONFIG_ESP32S3_SPI3
                {
                  static struct spi_dev_s * spi;
                  static bool in_use[2];
                  spi_p = &spi;
                  in_use_p = &in_use[cs_number];
                  port = ESP32S3_SPI3;
                  pins->minor_output = 3 + 2 * cs_number;
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
                if(!(*spi_p))
                  {
                    *spi_p = esp32s3_spibus_initialize(port);
                    if (!(*spi_p))
                      {
                        return -ENODEV;
                      }
                  }

                ret = spi_register(*spi_p, pins->minor_output);
                break;
#else /* CONFIG_SPI_DRIVER */
                return -ENODEV;
#endif

              case MCP_PINS_DRIVER_TYPE_SPI_SDCARD:
#ifdef CONFIG_MMCSD_SPI
                {
                  static uint8_t slot;
                  static bool in_use_by[2];
                  if(slot >= CONFIG_MMCSD_NSLOTS || in_use_by[port_idx])
                    {
                      return -ENODEV;
                    }

                  if(!(*spi_p))
                  {
                    *spi_p = esp32s3_spibus_initialize(port);
                    if (!(*spi_p))
                      {
                        return -ENODEV;
                      }
                  }

                  ret = mmcsd_spislotinitialize(pins->minor_output, slot, *spi_p);
                  if(ret != OK)
                    {
                      return ret;
                    }

                  slot++;
                  in_use_by[port_idx] = true;
                }
                break;
#else /* CONFIG_MMCSD_SPI */
                return -ENODEV;
#endif

              default:
                return -EINVAL;
            }

          if(ret == OK)
            {
              g_spi_driver_type[port_idx][cs_number] = pins->driver;
              g_spi_devid_hints[port_idx][cs_number] = pins->user_devid_hint;

              uint8_t cs_pinno = g_cs_pins[port_idx][cs_number];
              esp32s3_gpiowrite(cs_pinno, true);
              esp32s3_configgpio(cs_pinno, OUTPUT);
              // esp32s3_gpio_matrix_out(cs_pinno, SIG_GPIO_OUT_IDX, 0, 0);
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
