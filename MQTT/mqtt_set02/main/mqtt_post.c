#include "mqtt_post.h"
#include "esp_log.h"
#include "esp_random.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "mqtt_cam_publish";
// 按OneNET规范发布cam属性（解决code:2409）
esp_err_t mqtt_publish_cam_struct_onenet(esp_mqtt_client_handle_t client,
                                         const char *topic,
                                         const char *id,
                                         const char *cam1_val,
                                         const char *cam2_val,
                                         const char *cam3_val,
                                         const char *cam4_val,
                                         const char *cam5_val)
{
    if (client == NULL || topic == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // 处理空值
    const char *c1 = cam1_val ? cam1_val : "";
    const char *c2 = cam2_val ? cam2_val : "";
    const char *c3 = cam3_val ? cam3_val : "";
    const char *c4 = cam4_val ? cam4_val : "";
    const char *c5 = cam5_val ? cam5_val : "";

    // ========== 关键修改：按OneNET对象属性规范构建JSON ==========
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "id", id ? id : "3"); // 匹配平台指令的id
    cJSON_AddStringToObject(root, "version", "1.0");

    // params层：cam是对象类型，需用value包裹
    cJSON *params = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "params", params);

    // cam属性：{ "cam": { "value": { "cam1": "...", ... } } }
    cJSON *cam = cJSON_CreateObject();
    cJSON_AddItemToObject(params, "cam", cam);

    // 核心：添加value字段（OneNET对象类型属性必填）
    cJSON *cam_value = cJSON_CreateObject();
    cJSON_AddItemToObject(cam, "value", cam_value);

    // 填充cam1~cam5
    cJSON_AddStringToObject(cam_value, "cam1", c1);
    cJSON_AddStringToObject(cam_value, "cam2", c2);
    cJSON_AddStringToObject(cam_value, "cam3", c3);
    cJSON_AddStringToObject(cam_value, "cam4", c4);
    cJSON_AddStringToObject(cam_value, "cam5", c5);

    // 生成JSON字符串并打印
    char *json_pretty = cJSON_Print(root);
    char *json_str = cJSON_PrintUnformatted(root);
    ESP_LOGI(TAG, "【OneNET规范格式】:\n%s", json_pretty);

    // 发布消息
    int msg_id = esp_mqtt_client_publish(client, topic, json_str, 0, 1, 0);

    // 释放资源
    cJSON_free(json_pretty);
    cJSON_free(json_str);
    cJSON_Delete(root);

    return msg_id >= 0 ? ESP_OK : ESP_FAIL;
}