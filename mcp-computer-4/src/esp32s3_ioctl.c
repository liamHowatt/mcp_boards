#include <nuttx/config.h>

#ifdef CONFIG_BOARDCTL_IOCTL

#include <nuttx/board.h>
#include <arch/board/boardctl.h>
#include <errno.h>

#include "esp32s3-devkit.h"

/****************************************************************************
 * Name: boardctl
 *
 * Description:
 *   In a small embedded system, there will typically be a much greater
 *   interaction between application and low-level board features.  The
 *   canonically correct to implement such interactions is by implementing a
 *   character driver and performing the interactions via low level ioctl
 *   calls.  This, however, may not be practical in many cases and will lead
 *   to "correct" but awkward implementations.
 *
 *   boardctl() is non-standard OS interface to alleviate the problem.
 *   It basically circumvents the normal device driver ioctl interlace and
 *   allows the application to perform direct IOCTL-like calls to the
 *   board-specific logic.
 *   It is especially useful for setting up board operational and
 *   test configurations.
 *
 * Input Parameters:
 *   cmd - Identifies the board command to be executed
 *   arg - The argument that accompanies the command.  The nature of the
 *         argument is determined by the specific command.
 *
 * Returned Value:
 *   On success zero (OK) is returned; -1 (ERROR) is returned on failure
 *   with the errno variable to indicate the nature of the failure.
 *
 ****************************************************************************/

int board_ioctl(unsigned int cmd, uintptr_t arg)
{
  switch (cmd)
    {
#ifdef CONFIG_MCP_PINS
      case BIOC_MCP_PINS:
        return board_mcp_pins(arg);
#endif
      default:
        return -ENOTTY;
    }

  return OK;
}


#endif /* CONFIG_BOARDCTL_IOCTL */
