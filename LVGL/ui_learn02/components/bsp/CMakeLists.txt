idf_component_register(
    SRCS "cst816t_driver.c" "st7789.c"
    INCLUDE_DIRS "."
    REQUIRES
    esp_timer
    driver
    lvgl
    esp_lcd)
