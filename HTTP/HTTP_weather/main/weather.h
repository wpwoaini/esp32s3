#ifndef __WEATHER_H__
#define __WEATHER_H__

#include "esp_err.h"

// 天气预报结构体（仅保留需要的字段）
typedef struct
{
    char date[16];        // 日期
    int temp_max;         // 最高温度
    int temp_min;         // 最低温度
    char weather_day[16]; // 白天天气
} weather_forecast_day_t;

// 天气数据结构体（仅保留区县和前三天预报）
typedef struct
{
    char district[16];                  // 区县
    weather_forecast_day_t forecast[3]; // 前三天预报
} weather_data_t;

/**
 * @brief 获取天气数据（解析区县+前三天核心数据）
 * @param[out] weather_data 存储解析后的天气数据
 * @return ESP_OK 成功，其他值 失败
 */
esp_err_t weather_get_data(weather_data_t *weather_data);

#endif // __WEATHER_H__