#ifndef LV_PORT_H
#define LV_PORT_H

#include "lvgl.h"
#include "esp_err.h"
#include "cst816t_driver.h"
#include "st7789.h"

#define MY_DISP_HOR_RES 284
#define MY_DISP_VER_RES 240

// // ST7789 引脚
// #define GPIO_MOSI 5
// #define GPIO_SCLK 2
// #define GPIO_CS 4
// #define GPIO_DC 42
// #define GPIO_RST 6
// #define GPIO_BL 1
// // CST816T 引脚
// #define GPIO_SDA 40
// #define GPIO_SCL 41

void lv_port_disp_init(void);

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void);

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void);

esp_err_t lv_port_init(void);

void lv_port_indev_init(void);
#endif