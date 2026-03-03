#include <stdio.h>
#include "led_ws2812.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void app_main(void)
{
    ws2812_strip_handle_t ws_2812_handle = NULL;
    ws2812_init(GPIO_NUM_47, 1, &ws_2812_handle);
    ws2812_write(ws_2812_handle, 0, 10, 0, 0); // io_47
    vTaskDelay(pdMS_TO_TICKS(3000));
    ws2812_deinit(ws_2812_handle);
    ws2812_init(GPIO_NUM_48, 1, &ws_2812_handle);
    ws2812_write(ws_2812_handle, 0, 0, 0, 10); // io_47
    while (1)
    {
        /* code */
    }
}
