#include "mqtt_al.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "mbedtls/md5.h"
#include "mbedtls/md.h"
#include "esp_wifi.h"
#include "stdio.h"
#include "string.h"

#include "onenet_dm.h"

esp_mqtt_client_handle_t s_mqtt_client = NULL;
static const char *TAG = "ONENET";
static int s_is_mqtt_connected = 0;
void onenet_ack(int code, const char *msg);
extern const char *g_onenet_ca;
static void aliot_mqtt_event_handler(void *event_handler_arg,
                                     esp_event_base_t event_base,
                                     int32_t event_id,
                                     void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    // your_context_t *context = event->context;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED: // 连接成功
        ESP_LOGI(TAG, "mqtt connected");
        s_is_mqtt_connected = 1;
        // 连接成功后，订阅测试主题
        esp_mqtt_client_subscribe_single(s_mqtt_client, MQTT_POST_REPLY, 1);
        esp_mqtt_client_subscribe_single(s_mqtt_client, MQTT_SET, 1);
        break;
    case MQTT_EVENT_DISCONNECTED: // 连接断开
        ESP_LOGI(TAG, "mqtt disconnected");
        s_is_mqtt_connected = 0;
        break;
    case MQTT_EVENT_SUBSCRIBED: // 收到订阅消息ACK
        ESP_LOGI(TAG, " mqtt subscribed ack, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED: // 收到解订阅消息ACK
        break;
    case MQTT_EVENT_PUBLISHED: // 收到发布消息ACK
        ESP_LOGI(TAG, "mqtt publish ack, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "TOPIC->%.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG, "Payload->%.*s", event->data_len, event->data);

        if (strstr(event->topic, "/property/set"))
        {
            cJSON *property_js = cJSON_Parse(event->data);
            if (property_js == NULL)
            {
                ESP_LOGE(TAG, "JSON parse error: %s", cJSON_GetErrorPtr());
                return;
            }
            cJSON *params_js = cJSON_GetObjectItem(property_js, "params");
            if (params_js)
            {
                cJSON *light_js = cJSON_GetObjectItem(params_js, "LED");
                if (light_js)
                {
                    int value = cJSON_IsTrue(light_js) ? 1 : 0;
                    // ESP_LOGI(TAG, "LED value: %d", value);
                    onenet_post_int("LED", value);
                }
                onenet_ack(200, "success");
            }
        }

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        break;
    }
}

// char*  get_clientid(void){
//     uint8_t mac[6];
//     static char client_id[32]={0};
//     esp_wifi_get_mac(WIFI_IF_STA,mac);
//     snprintf(client_id,sizeof(client_id),"%02X%02X%02X%02X%02X%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
//     return client_id;
// }

// void cal_md5(char* key,char* content,char* ST,unsigned char* output){
//     mbedtls_md_context_t md5_ctx;
//     const mbedtls_md_info_t *md_info=mbedtls_md_info_from_type(MBEDTLS_MD_MD5);

//     mbedtls_md_init(&md5_ctx);
//     mbedtls_md_setup(&md5_ctx,md_info,1);
//     mbedtls_md_hmac_starts(&md5_ctx,(const unsigned char*)key,strlen(key));
//     mbedtls_md_hmac_update(&md5_ctx,(const unsigned char*)content,strlen(content));
//     mbedtls_md_hmac_finish(&md5_ctx,output);
//     mbedtls_md_free(&md5_ctx);
// }

// 启动MQTT
void mqtt_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {0};
    mqtt_cfg.broker.address.uri = MQTT_URL;
    mqtt_cfg.broker.address.port = MQTT_PORT;

    // client_id
    char client_id[128];
    snprintf(client_id, sizeof(client_id), DEVICENAME);

    // username
    char username[128];
    snprintf(username, sizeof(username), USERNAME);

    // //password
    // char StringForSignature = "1782490738"  "\n"  "md5" "\n"  "products/J6e5BQ12WS/devices/d1" "\n"  "2018-10-31";
    // char sign_content[256];
    // snprintf(sign_content,sizeof(sign_content),"version=2018-10-31&res=products/J6e5BQ12WS/devices/d1&et=1782490738&method=md5&sign=%s%%3D"，);
    // unsigned char password_hex[16];
    // char password_str[33];

    // cal_md5(DEVICESECRET,sign_content,password_hex);

    // Client ID
    mqtt_cfg.credentials.client_id = client_id;
    // 用户名
    mqtt_cfg.credentials.username = username;
    // 密码
    mqtt_cfg.credentials.authentication.password = TOKEN;
    mqtt_cfg.broker.verification.certificate = g_onenet_ca;
    mqtt_cfg.broker.verification.skip_cert_common_name_check = true;
    ESP_LOGI(TAG, "mqtt connect->clientId:%s,username:%s,password:%s", mqtt_cfg.credentials.client_id, mqtt_cfg.credentials.username, mqtt_cfg.credentials.authentication.password);
    // 设置mqtt配置，返回mqtt操作句柄
    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    // 注册mqtt事件回调函数
    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, aliot_mqtt_event_handler, s_mqtt_client);
    // 启动mqtt连接
    esp_mqtt_client_start(s_mqtt_client);
}

// 上报int数据
void onenet_post_int(const char *name, int value)
{
    ONENET_DM_DES *dm = onenet_malloc_des(ONENET_POST);
    // 创建物模型结构体
    onenet_set_dm_int(dm, name, value);
    // 设置物模型属性
    onenet_dm_serialize(dm);
    // json字符串
    //  char topic[128];
    //  snprintf(topic,sizeof(topic),"%s",MQTT_POST);
    esp_mqtt_client_publish(s_mqtt_client, MQTT_POST, dm->dm_js_str, strlen(dm->dm_js_str), 1, 0);
    // dm->dm_js_str; // This line is unnecessary, so you can remove or comment it out
    ESP_LOGI("me", "%s", dm->dm_js_str);
    onenet_dm_free(dm);
}

// 上报属性回复
void onenet_ack(int code, const char *msg)
{
    ONENET_DM_DES *dm = onenet_malloc_des(ONENET_SET_ACK);
    // 创建物模型结构体
    onenet_set_dm_ack(dm, code, msg);
    // 设置物模型属性
    onenet_dm_serialize(dm);
    // ESP_LOGI("MQTT-ACK", "Generated JSON: %s", dm->dm_js_str);
    // json字符串
    // char topic[128];
    // snprintf(topic,sizeof(topic),"%s",MQTT_POST);
    esp_mqtt_client_publish(s_mqtt_client, MQTT_SET_REPLY, dm->dm_js_str, strlen(dm->dm_js_str), 1, 0);
    // dm->dm_js_str; // This line is unnecessary, so you can remove or comment it out
    onenet_dm_free(dm);
    if (!s_is_mqtt_connected)
    {
        ESP_LOGW(TAG, "MQTT not connected, cannot send ACK");
        onenet_dm_free(dm);
        return;
    }
}

// 是否连接
char is_mqtt_connected(void)
{
    return s_is_mqtt_connected;
}

const char *g_onenet_ca = "-----BEGIN CERTIFICATE-----\n"
                          "MIIDOzCCAiOgAwIBAgIJAPCCNfxANtVEMA0GCSqGSIb3DQEBCwUAMDQxCzAJBgNV"
                          "BAYTAkNOMQ4wDAYDVQQKDAVDTUlPVDEVMBMGA1UEAwwMT25lTkVUIE1RVFRTMB4X"
                          "DTE5MDUyOTAxMDkyOFoXDTQ5MDUyMTAxMDkyOFowNDELMAkGA1UEBhMCQ04xDjAM"
                          "BgNVBAoMBUNNSU9UMRUwEwYDVQQDDAxPbmVORVQgTVFUVFMwggEiMA0GCSqGSIb3"
                          "DQEBAQUAA4IBDwAwggEKAoIBAQC/VvJ6lGWfy9PKdXKBdzY83OERB35AJhu+9jkx"
                          "5d4SOtZScTe93Xw9TSVRKrFwu5muGgPusyAlbQnFlZoTJBZY/745MG6aeli6plpR"
                          "r93G6qVN5VLoXAkvqKslLZlj6wXy70/e0GC0oMFzqSP0AY74icANk8dUFB2Q8usS"
                          "UseRafNBcYfqACzF/Wa+Fu/upBGwtl7wDLYZdCm3KNjZZZstvVB5DWGnqNX9HkTl"
                          "U9NBMS/7yph3XYU3mJqUZxryb8pHLVHazarNRppx1aoNroi+5/t3Fx/gEa6a5PoP"
                          "ouH35DbykmzvVE67GUGpAfZZtEFE1e0E/6IB84PE00llvy3pAgMBAAGjUDBOMB0G"
                          "A1UdDgQWBBTTi/q1F2iabqlS7yEoX1rbOsz5GDAfBgNVHSMEGDAWgBTTi/q1F2ia"
                          "bqlS7yEoX1rbOsz5GDAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQAL"
                          "aqJ2FgcKLBBHJ8VeNSuGV2cxVYH1JIaHnzL6SlE5q7MYVg+Ofbs2PRlTiWGMazC7"
                          "q5RKVj9zj0z/8i3ScWrWXFmyp85ZHfuo/DeK6HcbEXJEOfPDvyMPuhVBTzuBIRJb"
                          "41M27NdIVCdxP6562n6Vp0gbE8kN10q+ksw8YBoLFP0D1da7D5WnSV+nwEIP+F4a"
                          "3ZX80bNt6tRj9XY0gM68mI60WXrF/qYL+NUz+D3Lw9bgDSXxpSN8JGYBR85BxBvR"
                          "NNAhsJJ3yoAvbPUQ4m8J/CoVKKgcWymS1pvEHmF47pgzbbjm5bdthlIx+swdiGFa"
                          "WzdhzTYwVkxBaU+xf/2w\n"
                          "-----END CERTIFICATE-----";
