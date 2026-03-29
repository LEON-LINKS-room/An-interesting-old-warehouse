idf_component_register(SRCS "./gps/gps_deal.c" "./web/web_server.c" "my_main.c"
                    PRIV_REQUIRES spi_flash
                    REQUIRES esp_driver_uart esp_driver_gpio esp_http_server nvs_flash esp_wifi esp_netif
                    INCLUDE_DIRS "." "./gps" "./web")
