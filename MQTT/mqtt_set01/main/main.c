#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "a_wifi.h"
#include "mqtt_al.h"
#include "driver/gpio.h"
#include "onenet_dm.h"

#include "mqtt_post.h"

// static const char* TAG = "main";

// 定义一个事件组，用于通知main函数WIFI连接成功
#define WIFI_CONNECT_BIT BIT0
static EventGroupHandle_t s_wifi_ev = NULL;

void wifi_event_handler(WIFI_EV_e ev)
{
    if (ev == WIFI_CONNECTED)
    {
        xEventGroupSetBits(s_wifi_ev, WIFI_CONNECT_BIT);
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS出现错误，执行擦除
        ESP_ERROR_CHECK(nvs_flash_erase());
        // 重新尝试初始化
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    s_wifi_ev = xEventGroupCreate();
    EventBits_t ev = 0;

    // 初始化WIFI，传入回调函数，用于通知连接成功事件
    wifi_sta_init(wifi_event_handler);
    // 初始化button和led
    //  xTaskCreatePinnedToCore(taskA,"taskA",4096,NULL,2,NULL,1);

    // 一直监听WIFI连接事件，直到WiFi连接成功后，才启动MQTT连接
    ev = xEventGroupWaitBits(s_wifi_ev, WIFI_CONNECT_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
    if (ev & WIFI_CONNECT_BIT)
    {
        mqtt_start();
    }

    // mqtt_publish_cam_struct_onenet(s_mqtt_client, MQTT_POST, "3", "1", "2", "3", "4", "5");
    const char *a = "2";
    const char *b = "4";
    const char *c = "6";
    const char *d = "8";
    const char *e = "10";
    mqtt_publish_cam_struct_onenet(s_mqtt_client, MQTT_POST, "3", a, b, c, d, e);
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
