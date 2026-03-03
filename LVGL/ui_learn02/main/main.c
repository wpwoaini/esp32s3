#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "lv_port.h"
#include "lv_demos.h"
#include "gui_guider.h"
#include "custom.h"
#include "wifi_manage.h"
#include "time.h"
#include "esp_sntp.h"
#include "weather.h"
// psram引脚为35，36，37

// #define PIN_NUM_CLK 2
// #define PIN_NUM_DC 42
// #define PIN_NUM_CS 4
// #define PIN_NUM_MOSI 5
// #define PIN_NUM_RST 6
// scl = GPIO_NUM_16,
// sda = GPIO_NUM_15,
lv_ui guider_ui;
static void lvgl_task(void *arg);
static void wifi_start_state(wifi_manage_event_t event);
static void sntp_sync_time_cb(struct timeval *tv);
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
    set_home_weather(&guider_ui, weather_data);
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

static void sntp_sync_time_cb(struct timeval *tv)
{
    // time_t my_time = time(NULL);
    // ESP_LOGI("time", "%lld\n", my_time);
    struct tm t;
    localtime_r(&tv->tv_sec, &t);
    set_home_time(&guider_ui, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_wday, t.tm_hour, t.tm_min, t.tm_sec);
}

void my_sntp_init(void)
{
    if (!esp_sntp_enabled())
    {
        esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "ntp.aliyun.com");
        esp_sntp_set_time_sync_notification_cb(sntp_sync_time_cb);
        esp_sntp_init();
        setenv("TZ", "CST-8", 1);
        tzset();
    }
}

void app_main(void)
{
    // 初始化 NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    // 初始化
    wifi_manage_init(wifi_start_state);
    wifi_manage_start();

    xTaskCreatePinnedToCore(lvgl_task, "lvgl_task", 10240, NULL, 5, NULL, 0);
    // main任务保持轻量，仅做监控或其他简单逻辑
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void lvgl_task(void *arg)
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

static void wifi_start_state(wifi_manage_event_t event)
{
    switch (event)
    {
    case WIFI_MANAGE_EVENT_STA_CONNECTED:
        ESP_LOGI("wifi", "wifi connected\n");
        break;
    case WIFI_MANAGE_EVENT_STA_DISCONNECTED:
        ESP_LOGI("wifi", "wifi disconnected\n");
        break;
    case WIFI_MANAGE_EVENT_STA_GOT_IP:
        ESP_LOGI("wifi", "wifi got ip\n");
        my_sntp_init();
        xTaskCreatePinnedToCore(weather_task, "weather_init", 4096, NULL, 5, &weather_handle, 1);
        break;
    case WIFI_MANAGE_EVENT_AP_START:
        ESP_LOGI("wifi", "ap start\n");
        break;
    default:
        ESP_LOGI("wifi", "unknown event\n");
        break;
    }
}