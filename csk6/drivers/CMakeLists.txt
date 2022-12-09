
if(${CONFIG_SOC_LS_ADSP_CSK6})
    # add_definitions(-DIC_BOARD=1)
    zephyr_compile_options(-DPLATFORM_CP -DCSK6001 -DIC_BOARD=1)

    # add files
    zephyr_include_directories(hifi4/xos 
                                hifi4/include
                                hifi4/include/hal
                                hifi4/include/hal/reg)
    zephyr_library_sources(hifi4/venus_cp.c
                            hifi4/dma_venus.c
                            hifi4/spi.c
                            hifi4/cache_cp.c
                            hifi4/gpio_venus.c
                            hifi4/uart.c
                            hifi4/mbox.c
                            hifi4/IOMuxManager.c
                            hifi4/ClockManager.c
                            hifi4/apc_venus.c
                            hifi4/dac_venus.c
                            hifi4/adc_pdm_venus.c)
elseif(${CONFIG_SOC_CSK6})
    zephyr_compile_options(-DARMCM30_DSP_FP -DPLATFORM_AP -DCSK6001 -DIC_BOARD)
    zephyr_include_directories(cm33/include)
    zephyr_include_directories(cm33/include/hal)
    zephyr_include_directories(cm33/include/hal/reg)
    zephyr_include_directories(cm33/include/hal/sdmmc)
    zephyr_library_sources(cm33/venus_ap.c
                            cm33/clock_config.c 
                            cm33/dma.c
                            cm33/cache_ap.c
                            cm33/gpio.c
                            cm33/uart.c
                            cm33/IOMuxManager.c
                            cm33/ClockManager.c
                            cm33/platform-castor.c
                            cm33/spiflash.c
                            cm33/gpt.c
                            cm33/gpt_pwm.c
                            cm33/mbox.c
                            cm33/PSRAMManger.c
                            cm33/i2c.c
                            cm33/spi.c
                            cm33/rtc.c
                            cm33/dual_timer.c
                            cm33/power_control.c
                            cm33/gpadc_venus.c
                            cm33/AON_IOMuxManager.c
                            cm33/keysense.c
                            cm33/dvp_venus.c
                            cm33/crypto.c
                            cm33/trng.c
                            cm33/sdmmc/drv_sdc.c
                            cm33/sdmmc/ftsdc021_ops.c
                            cm33/sdmmc/ftsdc021_sdio.c
                            cm33/sdmmc/ftsdc021.c
                            cm33/aon_timer.c
                            cm33/pwr.c
                            )
else()
    message(FATAL_ERROR 'listenai hal driver does not support this platform')
endif()


