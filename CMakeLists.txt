# Makefile - STM32Cube SDK
#
# Copyright (c) 2020 Linaro Limited
#
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
add_subdirectory_ifdef(CONFIG_HAS_CSK6001 csk6001)
