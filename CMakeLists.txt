
if(${CONFIG_SOC_CSK6})
    zephyr_compile_options(-DARMCM30_DSP_FP -DPLATFORM_AP -DCSK6001 -DIC_BOARD)
    zephyr_include_directories(include)
    zephyr_include_directories(include/hal)
    zephyr_include_directories(include/hal/reg)
    zephyr_include_directories(include/hal/sdmmc)
    zephyr_library_sources(source/venus_ap.c
                            source/clock_config.c 
                            source/dma.c
                            source/cache_ap.c
                            source/gpio.c
                            source/uart.c
                            source/IOMuxManager.c
                            source/ClockManager.c
                            source/platform-castor.c
                            source/spiflash.c
                            source/gpt.c
                            source/gpt_timer.c
                            source/gpt_pwm.c
                            source/mbox.c
                            source/PSRAMManger.c
                            source/i2c.c
                            source/ir.c
                            source/spi.c
                            source/rtc.c
                            source/dual_timer.c
                            source/power_control.c
                            source/gpadc_venus.c
                            source/AON_IOMuxManager.c
                            source/keysense.c
                            source/dvp_venus.c
                            source/crypto.c
                            source/trng.c
                            source/sdmmc/drv_sdc.c
                            source/sdmmc/ftsdc021_ops.c
                            source/sdmmc/ftsdc021_sdio.c
                            source/sdmmc/ftsdc021.c
                            source/aon_timer.c
                            source/pwr.c
                            )
else()
    message(FATAL_ERROR 'listenai hal driver does not support this platform')
endif()


