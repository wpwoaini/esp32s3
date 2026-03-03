#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "weather.h"
#include "wifi_manage.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static TaskHandle_t weather_handle = NULL;
void weather_init(void)
{
    // 定义存储天气数据的结构体
    weather_data_t weather_data = {0};

    //  获取天气数据（仅解析区县+前三天核心字段）
    esp_err_t err = weather_get_data(&weather_data);
    if (err != ESP_OK)
    {
        ESP_LOGE("weather", "Get weather data failed: %s", esp_err_to_name(err));
        return;
    }

    //  打印解析结果（仅区县+前三天核心数据）
    ESP_LOGI("weather", "========== 天气数据（%s）==========", weather_data.district);
    for (int i = 0; i < 3; i++)
    {
        ESP_LOGI("weather", "日期：%s", weather_data.forecast[i].date);
        ESP_LOGI("weather", "白天天气：%s", weather_data.forecast[i].weather_day);
        ESP_LOGI("weather", "温度范围：%d℃ ~ %d℃",
                 weather_data.forecast[i].temp_min,
                 weather_data.forecast[i].temp_max);
        ESP_LOGI("weather", "----------------------------------");
    }
}

void weather_task()
{
    while (1)
    {
        weather_init();
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        vTaskDelete(weather_handle);
    }
}

void wifi_manage_cb(wifi_manage_event_t event)
{
    switch (event)
    {
    case WIFI_MANAGE_EVENT_STA_CONNECTED:
        ESP_LOGI("wifi_manage", "Connected to Wi-Fi network");

        break;
    case WIFI_MANAGE_EVENT_STA_DISCONNECTED:
        ESP_LOGI("wifi_manage", "Disconnected from Wi-Fi network");
        break;
    case WIFI_MANAGE_EVENT_STA_GOT_IP:
        ESP_LOGI("wifi_manage", "Disconnected from Wi-Fi network");
        xTaskCreatePinnedToCore(weather_task, "weather_init", 4096, NULL, 5, &weather_handle, 1);
        break;
    default:
        break;
    }
}
void app_main(void)
{
    nvs_flash_init();
    wifi_manage_init(wifi_manage_cb);
    wifi_manage_start();
}
