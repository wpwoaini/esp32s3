#include "weather.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include <string.h>
#include "esp_crt_bundle.h"
static const char *TAG = "weather";

// 存储HTTP响应数据的临时缓冲区
static char *http_response_buf = NULL;
static int http_response_len = 0;

/**
 * @brief HTTP事件回调函数（仅处理响应数据）
 */
static esp_err_t weather_http_event_cb(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        // 非chunked模式下接收数据
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // 初始化缓冲区
            if (http_response_buf == NULL)
            {
                http_response_buf = (char *)malloc(evt->data_len + 1);
                if (http_response_buf == NULL)
                {
                    ESP_LOGE(TAG, "Malloc response buffer failed");
                    return ESP_ERR_NO_MEM;
                }
                http_response_len = 0;
            }
            else
            {
                // 扩容缓冲区
                char *tmp = (char *)realloc(http_response_buf, http_response_len + evt->data_len + 1);
                if (tmp == NULL)
                {
                    ESP_LOGE(TAG, "Realloc response buffer failed");
                    free(http_response_buf);
                    http_response_buf = NULL;
                    return ESP_ERR_NO_MEM;
                }
                http_response_buf = tmp;
            }
            // 拷贝数据并添加字符串结束符
            memcpy(http_response_buf + http_response_len, evt->data, evt->data_len);
            http_response_len += evt->data_len;
            http_response_buf[http_response_len] = '\0';
        }
        break;

    case HTTP_EVENT_DISCONNECTED:
        // 断开连接时清理缓冲区
        if (http_response_buf)
        {
            free(http_response_buf);
            http_response_buf = NULL;
            http_response_len = 0;
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}

/**
 * @brief 解析JSON数据（仅提取区县和前三天核心预报）
 */
static esp_err_t weather_parse_json(const char *json_str, weather_data_t *weather_data)
{
    if (json_str == NULL || weather_data == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // 清空结构体，避免脏数据
    memset(weather_data, 0, sizeof(weather_data_t));

    // 解析JSON根节点
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL)
    {
        ESP_LOGE(TAG, "JSON parse failed: %s", cJSON_GetErrorPtr());
        return ESP_FAIL;
    }

    // ========== 仅解析区县（district） ==========
    cJSON *district = cJSON_GetObjectItem(root, "district");
    if (district && cJSON_IsString(district) && district->valuestring != NULL)
    {
        strncpy(weather_data->district, district->valuestring, sizeof(weather_data->district) - 1);
        weather_data->district[sizeof(weather_data->district) - 1] = '\0'; // 确保字符串结束
    }
    else
    {
        ESP_LOGE(TAG, "Parse district failed");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    // ========== 仅解析前三天预报核心字段 ==========
    cJSON *forecast_arr = cJSON_GetObjectItem(root, "forecast");
    if (!cJSON_IsArray(forecast_arr))
    {
        ESP_LOGE(TAG, "Forecast is not an array");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    // 提取前3天数据（若不足3天则取实际数量）
    int forecast_count = cJSON_GetArraySize(forecast_arr);
    int parse_days = (forecast_count > 3) ? 3 : forecast_count;
    for (int i = 0; i < parse_days; i++)
    {
        cJSON *day_item = cJSON_GetArrayItem(forecast_arr, i);
        if (day_item == NULL)
            continue;

        // 日期
        cJSON *date = cJSON_GetObjectItem(day_item, "date");
        if (date && cJSON_IsString(date))
        {
            strncpy(weather_data->forecast[i].date, date->valuestring, sizeof(weather_data->forecast[i].date) - 1);
            weather_data->forecast[i].date[sizeof(weather_data->forecast[i].date) - 1] = '\0';
        }

        // 最高温度
        cJSON *temp_max = cJSON_GetObjectItem(day_item, "temp_max");
        if (temp_max && cJSON_IsNumber(temp_max))
        {
            weather_data->forecast[i].temp_max = temp_max->valueint;
        }

        // 最低温度
        cJSON *temp_min = cJSON_GetObjectItem(day_item, "temp_min");
        if (temp_min && cJSON_IsNumber(temp_min))
        {
            weather_data->forecast[i].temp_min = temp_min->valueint;
        }

        // 白天天气
        cJSON *weather_day = cJSON_GetObjectItem(day_item, "weather_day");
        if (weather_day && cJSON_IsString(weather_day))
        {
            strncpy(weather_data->forecast[i].weather_day, weather_day->valuestring, sizeof(weather_data->forecast[i].weather_day) - 1);
            weather_data->forecast[i].weather_day[sizeof(weather_data->forecast[i].weather_day) - 1] = '\0';
        }
    }

    // 释放JSON内存
    cJSON_Delete(root);
    return ESP_OK;
}

/**
 * @brief 对外接口：获取天气数据
 */
esp_err_t weather_get_data(weather_data_t *weather_data)
{
    if (weather_data == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // 重置缓冲区
    if (http_response_buf)
    {
        free(http_response_buf);
        http_response_buf = NULL;
        http_response_len = 0;
    }

    // 配置HTTP客户端（指定攸县，获取预报）
    esp_http_client_config_t http_cfg = {
        .url = "https://uapis.cn/api/v1/misc/weather?forecast=true",
        .method = HTTP_METHOD_GET,
        .event_handler = weather_http_event_cb,
        .skip_cert_common_name_check = true,
        .timeout_ms = 10000, // 10秒超时
        .auth_type = HTTP_AUTH_TYPE_BASIC,
        .use_global_ca_store = true,
        .skip_cert_common_name_check = true,
        .crt_bundle_attach = esp_crt_bundle_attach,

        // .crt_bundle_attach = esp_crt_bundle_attach,
    };

    // 创建并初始化HTTP客户端
    esp_http_client_handle_t client = esp_http_client_init(&http_cfg);
    if (client == NULL)
    {
        ESP_LOGE(TAG, "Create HTTP client failed");
        return ESP_FAIL;
    }

    // 执行HTTP请求
    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "HTTP perform failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }

    // 检查响应状态码
    int status_code = esp_http_client_get_status_code(client);
    if (status_code != 200)
    {
        ESP_LOGE(TAG, "HTTP request failed, status code: %d", status_code);
        esp_http_client_cleanup(client);
        free(http_response_buf);
        http_response_buf = NULL;
        return ESP_FAIL;
    }

    // 解析JSON数据
    err = weather_parse_json(http_response_buf, weather_data);

    // 清理资源
    esp_http_client_cleanup(client);
    free(http_response_buf);
    http_response_buf = NULL;
    http_response_len = 0;

    return err;
}