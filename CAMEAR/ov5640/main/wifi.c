#include <stdio.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "wifi.h"
#include "esp_event.h"
#include "esp_netif.h" // 必须包含（esp_ip4addr_ntoa依赖）
#include "string.h"    // strcpy依赖
static const char *TAG = "ESP32摄像头";

// WiFi事件处理
void wifi_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG, "WiFi断开连接，正在尝试重新连接...");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *get_ip = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "获取到IP地址: " IPSTR, IP2STR(&get_ip->ip_info.ip));
        // char ip[16] = {0}; // 存储获取到的IP地址
        // const char *ip_str = esp_ip4addr_ntoa(&event->ip_info.ip, ip, sizeof(ip));
        // strcpy(ip, ip_str);
        // ESP_LOGI("test", "ip:%s", ip);
    }
}

// 初始化WiFi连接
void wifi_init()
{
    ESP_ERROR_CHECK(esp_netif_init());
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 配置WiFi连接参数
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // 注册事件处理器
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_start());
}
void wifi_start()
{
    // 连接WiFi
    wifi_init();

    // 等待获取IP地址
    esp_netif_ip_info_t ip_info;
    int retry = 0;
    while (retry++ < 20)
    {
        if (esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info) == ESP_OK)
        {
            if (ip_info.ip.addr != 0)
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (ip_info.ip.addr == 0)
    {
        ESP_LOGE(TAG, "获取IP地址失败!");
        esp_restart();
    }
}