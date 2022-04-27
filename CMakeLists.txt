# Makefile - STM32Cube SDK
#
# Copyright (c) 2020 Linaro Limited
#
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_cc_option(-O0)
add_subdirectory_ifdef(CONFIG_HAS_CSK6 csk6)
