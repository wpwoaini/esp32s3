#include <stdio.h>
#include "lv_port.h"
#include "lv_demos.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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
// psram引脚为35，36，37

void lvgl_task(void *arg)
{
    // LVGL初始化和界面逻辑放到独立任务
    lv_port_init(); // 显示接口初始化（耗时且占栈）
    st7789_lcd_backlight(1);
    st7789_lcd_set_backlight(50);
    lv_demo_widgets();
    // ui_home_create();
    while (1)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{

    // 创建LVGL独立任务，指定足够栈空间（如10240字节）
    // xTaskCreate(
    //     lvgl_task,
    //     "lvgl_task",
    //     10240, // 栈大小可根据实际需求调整
    //     NULL,
    //     5, // 优先级低于main（可选）
    //     NULL);
    xTaskCreatePinnedToCore(lvgl_task, "lvgl_task", 10240, NULL, 5, NULL, 0);
    // main任务保持轻量，仅做监控或其他简单逻辑
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
