idf_component_register(SRCS "cli_lite.c" "demo_main.c"
                    PRIV_REQUIRES spi_flash
                    REQUIRES esp_driver_uart esp_driver_gpio
                    INCLUDE_DIRS ".")
