/****************************************************************************
 * boards/xtensa/esp32s3/esp32s3-devkit/src/esp32s3_gpio.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <nuttx/irq.h>
#include <arch/irq.h>
#include <assert.h>
#include <debug.h>
#include <stdio.h>

#include <nuttx/ioexpander/gpio.h>

#include <arch/board/board.h>

#include "esp32s3-devkit.h"
#include "esp32s3_gpio.h"
#include "hardware/esp32s3_gpio_sigmap.h"

#if defined(CONFIG_DEV_GPIO) && !defined(CONFIG_GPIO_LOWER_HALF)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#if !defined(CONFIG_ESP32S3_GPIO_IRQ)
#  error "GPIO interrupts aren't enabled"
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct gp_s
{
  struct gpio_dev_s dev;
  unsigned pin;
  pin_interrupt_t interrupt_callback;
  gpio_intrtype_t interrupt_type;
};

struct mcp_s
{
  struct gp_s half[2];
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int gp_read(struct gpio_dev_s *dev, bool *value);
static int gp_write(struct gpio_dev_s *dev, bool value);
static int gp_attach(struct gpio_dev_s *dev,
                     pin_interrupt_t callback);
static int gp_enable(struct gpio_dev_s *dev, bool enable);
static int gp_setpintype_mcp(FAR struct gpio_dev_s *dev,
                             enum gpio_pintype_e pintype);
static int gp_setpintype_gpioin(FAR struct gpio_dev_s *dev,
                                enum gpio_pintype_e pintype);

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const struct gpio_operations_s gp_ops_mcp =
{
  .go_read   = gp_read,
  .go_write  = gp_write,
  .go_attach = gp_attach,
  .go_enable = gp_enable,
  .go_setpintype = gp_setpintype_mcp,
};

static const struct gpio_operations_s gp_ops_gpioin =
{
  .go_read   = gp_read,
  .go_write  = NULL,
  .go_attach = gp_attach,
  .go_enable = gp_enable,
  .go_setpintype = gp_setpintype_gpioin,
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: gp_read
 ****************************************************************************/

static int gp_read(struct gpio_dev_s *dev, bool *value)
{
  struct gp_s *gp = (struct gp_s *)dev;

  DEBUGASSERT(gp != NULL && value != NULL);
  gpioinfo("Reading... pin %u\n", gp->pin);

  *value = esp32s3_gpioread(gp->pin);
  return OK;
}

/****************************************************************************
 * Name: gp_write
 ****************************************************************************/

static int gp_write(struct gpio_dev_s *dev, bool value)
{
  gpioinfo("Writing %d\n", (int)value);
  return value == 0 ? OK : ERROR;
}

/****************************************************************************
 * Name: esp32s3gpio_interrupt
 ****************************************************************************/

static int esp32s3gpio_interrupt(int irq, void *context, void *arg)
{
  struct gp_s *gp = (struct gp_s *)arg;

  gp->interrupt_callback(&gp->dev, gp->pin);
  return OK;
}

/****************************************************************************
 * Name: gp_attach
 ****************************************************************************/

static int gp_attach(struct gpio_dev_s *dev,
                     pin_interrupt_t callback)
{
  struct gp_s *gp = (struct gp_s *)dev;
  int irq = ESP32S3_PIN2IRQ(gp->pin);
  int ret;

  gpioinfo("Attaching the callback\n");

  /* Make sure the interrupt is disabled */

  esp32s3_gpioirqdisable(irq);
  ret = irq_attach(irq,
                   esp32s3gpio_interrupt,
                   gp);
  if (ret < 0)
  {
    syslog(LOG_ERR, "ERROR: gp_attach() failed: %d\n", ret);
    return ret;
  }

  gpioinfo("Attach %p\n", callback);
  gp->interrupt_callback = callback;
  return OK;
}

/****************************************************************************
* Name: gp_enable
****************************************************************************/

static int gp_enable(struct gpio_dev_s *dev, bool enable)
{
  struct gp_s *gp = (struct gp_s *)dev;
  int irq = ESP32S3_PIN2IRQ(gp->pin);

  if (enable)
  {
    if (gp->interrupt_callback != NULL)
    {
      gpioinfo("Enabling the interrupt\n");

      /* Configure the interrupt */

      esp32s3_gpioirqenable(irq, gp->interrupt_type);
    }
  }
  else
  {
    gpioinfo("Disable the interrupt\n");
    esp32s3_gpioirqdisable(irq);
  }

  return OK;
}

/****************************************************************************
 * Name: gp_setpintype
 ****************************************************************************/

static int gp_setpintype_mcp(FAR struct gpio_dev_s *dev,
                             enum gpio_pintype_e pintype)
{
  struct gp_s *half = (struct gp_s *)dev;

  DEBUGASSERT(half != NULL);
  gpioinfo("Setting type %d... pin %u\n", pintype, half->pin);

  switch (pintype) {
    case GPIO_INTERRUPT_RISING_PIN:
      esp32s3_configgpio(half->pin, INPUT_PULLUP);
      break;
    case GPIO_OUTPUT_PIN:
      int irq = ESP32S3_PIN2IRQ(half->pin);
      esp32s3_gpioirqdisable(irq);
      esp32s3_configgpio(half->pin, OUTPUT);
      break;
    default:
      return ERROR;
  }

  half->dev.gp_pintype = pintype;

  return OK;
}

static int gp_setpintype_gpioin(FAR struct gpio_dev_s *dev,
                                enum gpio_pintype_e pintype)
{
  struct gp_s *gp = (struct gp_s *)dev;

  DEBUGASSERT(gp != NULL);
  gpioinfo("Setting type %d... pin %u\n", pintype, gp->pin);

  switch (pintype) {
    case GPIO_INPUT_PIN:
      gp->interrupt_type = DISABLED; /* should never actually be set */
      break;
    case GPIO_INTERRUPT_RISING_PIN:
      gp->interrupt_type = RISING;
      break;
    case GPIO_INTERRUPT_FALLING_PIN:
      gp->interrupt_type = FALLING;
      break;
    case GPIO_INTERRUPT_BOTH_PIN:
      gp->interrupt_type = CHANGE;
      break;
    default:
      return ERROR;
  }

  /* even if going from interrupt type to interrupt type,
   * still need to reattach the new type
   */

  int irq = ESP32S3_PIN2IRQ(gp->pin);
  esp32s3_gpioirqdisable(irq);

  gp->dev.gp_pintype = pintype;

  return OK;
}


static void register_mcp_half(struct gp_s *half, unsigned idx, unsigned pin, bool is_clk)
{
  half->dev.gp_pintype = GPIO_INTERRUPT_RISING_PIN;
  half->dev.gp_ops = &gp_ops_mcp;
  half->pin = pin;
  half->interrupt_callback = NULL;
  half->interrupt_type = RISING;

  esp32s3_gpiowrite(pin, 0);
  esp32s3_configgpio(pin, INPUT_PULLUP);

  char name[16];
  snprintf(name, sizeof(name), "mcp%u_%s", idx, is_clk ? "clk" : "dat");

  gpio_pin_register_byname(&half->dev, name);
}

static void register_mcp(struct mcp_s *mcp, unsigned idx, unsigned clk_pin, unsigned dat_pin)
{
  register_mcp_half(&mcp->half[0], idx, clk_pin, true);
  register_mcp_half(&mcp->half[1], idx, dat_pin, false);
}


static void register_gpioin(struct gp_s * gp, unsigned idx, unsigned pin)
{
  gp->dev.gp_pintype = GPIO_INPUT_PIN;
  gp->dev.gp_ops = &gp_ops_gpioin;
  gp->pin = pin;
  gp->interrupt_callback = NULL;
  gp->interrupt_type = DISABLED;

  esp32s3_gpiowrite(pin, 0);
  esp32s3_configgpio(pin, INPUT);

  char name[16];
  snprintf(name, sizeof(name), "gpioin%u", idx);

  gpio_pin_register_byname(&gp->dev, name);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: esp32s3_gpio_init
 ****************************************************************************/

int esp32s3_gpio_init(void)
{
  static struct mcp_s mcp_0;
  register_mcp(&mcp_0, 0, 21, 14);
  static struct mcp_s mcp_1;
  register_mcp(&mcp_1, 1, 16, 15);

  static struct gp_s gpioin_0;
  register_gpioin(&gpioin_0, 0, 10);

  return OK;
}

#endif /* CONFIG_DEV_GPIO && !CONFIG_GPIO_LOWER_HALF */
