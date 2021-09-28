zephyr_include_directories(. include)
zephyr_library_sources(venus_ap.c clock_config.c dma.c cache_ap.c
                        gpio.c uart.c
                        IOMuxManager.c ClockManager.c)
