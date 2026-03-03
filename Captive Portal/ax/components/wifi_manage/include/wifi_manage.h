#ifndef __WIFI_MANAGE_H__
#define __WIFI_MANAGE_H__

#include "esp_err.h"

/**
 * @brief WiFi配置状态事件枚举
 * 用于通知外部程序WiFi的各种状态变化
 */
typedef enum
{
    WIFI_MANAGE_EVENT_AP_START = 0,     // AP模式启动成功
    WIFI_MANAGE_EVENT_AP_STOP,          // AP模式已停止
    WIFI_MANAGE_EVENT_STA_START,        // Station模式启动成功
    WIFI_MANAGE_EVENT_STA_CONNECTED,    // 已连接到目标WiFi
    WIFI_MANAGE_EVENT_STA_DISCONNECTED, // 与目标WiFi的连接断开
    WIFI_MANAGE_EVENT_STA_GOT_IP,       // 获取到IP地址
} wifi_manage_event_t;

/**
 * @brief WiFi状态回调函数类型定义
 * @param event WiFi状态事件
 */
typedef void (*wifi_manage_cb_t)(wifi_manage_event_t event);

/**
 * @brief 初始化WiFi管理器
 *
 * @param cb 状态回调函数，用于接收WiFi状态变化通知
 * @return esp_err_t
 *         - ESP_OK: 初始化成功
 *         - 其他: 初始化失败
 */
esp_err_t wifi_manage_init(wifi_manage_cb_t cb);

/**
 * @brief 启动WiFi配网功能
 *
 * @return esp_err_t
 *         - ESP_OK: 启动成功
 *         - 其他: 启动失败
 */
esp_err_t wifi_manage_start(void);

/**
 * @brief WiFi扫描结果结构体
 */
typedef struct
{
    char ssid[33];    // WiFi名称
    int8_t rssi;      // 信号强度
    uint8_t authmode; // 认证模式
} wifi_scan_result_t;

/**
 * @brief 扫描周围WiFi
 *
 * @return esp_err_t
 */
esp_err_t wifi_manage_scan(void);

/**
 * @brief 断开当前WiFi连接
 *
 * @return esp_err_t
 */
esp_err_t wifi_manage_disconnect(void);

/**
 * @brief 获取WiFi连接状态
 *
 * @return uint8_t
 *         - 0: 未连接
 *         - 1: 已连接
 */
uint8_t wifi_manage_get_status(void);

/**
 * @brief 获取MAC地址
 *
 * @param mac 存储MAC地址的缓冲区
 * @return esp_err_t
 */
esp_err_t wifi_manage_get_mac(uint8_t *mac);

/**
 * @brief 恢复AP模式（STA模式保持不变）
 * @return esp_err_t 错误码，ESP_OK表示成功
 */
esp_err_t resume_ap_keep_sta(void);

/**
 * @brief 停止AP模式，保留STA模式继续工作
 * @return esp_err_t 错误码，ESP_OK表示成功
 */
esp_err_t stop_ap_keep_sta(void);
#endif // __WIFI_MANAGE_H__