kconfig-tweak --disable CONFIG_NDEBUG
kconfig-tweak --enable CONFIG_ARCH_BOARD_CUSTOM
kconfig-tweak --set-str CONFIG_ARCH_BOARD_CUSTOM_DIR ../mcp_boards/mcp-computer-4
kconfig-tweak --enable CONFIG_ARCH_BOARD_CUSTOM_DIR_RELPATH
kconfig-tweak --set-str CONFIG_ARCH_BOARD_CUSTOM_NAME mcp-computer-4
make oldconfig
