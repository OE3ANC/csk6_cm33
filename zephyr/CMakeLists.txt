# Makefile - STM32Cube SDK
#
# Copyright (c) 2020 Linaro Limited
#
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_cc_option(-O0)
add_subdirectory_ifdef(CONFIG_CSK_HEAP ../zephyr/heap)

if(${CONFIG_SOC_CSK6})
    zephyr_compile_options(-DARMCM30_DSP_FP -DPLATFORM_AP -DCSK6001 -DIC_BOARD)
    zephyr_include_directories(./)
    zephyr_include_directories(../..)
    zephyr_include_directories(..)
    zephyr_include_directories(../include)
    zephyr_include_directories(../include/reg)
    zephyr_include_directories(../include/sdmmc)
    zephyr_library_sources(../src/venus_ap.c
                            ../src/clock_config.c 
                            ../src/dma.c
                            ../src/cache_ap.c
                            ../src/gpio.c
                            ../src/uart.c
                            ../src/IOMuxManager.c
                            ../src/ClockManager.c
                            ../src/platform-castor.c
                            ../src/spiflash.c
                            ../src/gpt.c
                            ../src/gpt_timer.c
                            ../src/gpt_pwm.c
                            ../src/mbox.c
                            ../src/PSRAMManger.c
                            ../src/i2c.c
                            ../src/ir.c
                            ../src/spi.c
                            ../src/rtc.c
                            ../src/dual_timer.c
                            ../src/power_control.c
                            ../src/gpadc_venus.c
                            ../src/AON_IOMuxManager.c
                            ../src/keysense.c
                            ../src/dvp_venus.c
                            ../src/crypto.c
                            ../src/trng.c
                            ../src/sdmmc/drv_sdc.c
                            ../src/sdmmc/ftsdc021_ops.c
                            ../src/sdmmc/ftsdc021_sdio.c
                            ../src/sdmmc/ftsdc021.c
                            ../src/aon_timer.c
                            ../src/pwr.c
                            ../src/wdt.c
                            )


zephyr_compile_options(-DHAL_CUSTOM_CONFIG_FILE="hal_config.h")

if (CONFIG_PSRAM_SIZE EQUAL 0x400000)
    message(STATUS "psram size: 4M")
    zephyr_compile_options(-DPSRAM_CHIP_SIZE=4)
else()
    message(STATUS "psram size: 8M")
endif()

endif()


