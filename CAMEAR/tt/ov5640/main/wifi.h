#ifndef _WIFI_H
#define _WIFI_H

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"

// 配置WiFi信息（保持不变）
#define WIFI_SSID "Xiaomi_User"
#define WIFI_PASSWORD "996633aa"
// extern char ip[16]; // 存储获取到的IP地址
void wifi_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data);
void wifi_init();
void wifi_start(); // 启动WIFI
#endif