file(GLOB_RECURSE CLOCK_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.c")
idf_component_register(SRCS ${CLOCK_SOURCES}
    INCLUDE_DIRS "." "board" "display" "clock" "faces" "time" "touch" "settings"
    REQUIRES espressif__esp_lcd_co5300 waveshare__esp_lcd_touch_cst9217
    esp_lcd esp_driver_spi esp_driver_gpio esp_driver_i2c esp_driver_usb_serial_jtag esp_timer
    nvs_flash esp_wifi esp_netif esp_event lwip esp_http_server spi_flash)
