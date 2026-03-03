#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lv_port.h"
#include "lv_demos.h"
#include "gui_guider.h"
#include "custom.h"

// psram引脚为35，36，37
lv_ui guider_ui;

void lvgl_task(void *arg)
{
    // LVGL初始化和界面逻辑放到独立任务
    lv_port_init(); // 显示接口初始化（耗时且占栈）
    // st7789_lcd_backlight(1);
    st7789_lcd_set_backlight(50);
    // lv_demo_widgets();
    setup_ui(&guider_ui);
    custom_init(&guider_ui);
    while (1)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    xTaskCreatePinnedToCore(lvgl_task, "lvgl_task", 10240, NULL, 5, NULL, 0);
    // main任务保持轻量，仅做监控或其他简单逻辑
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
