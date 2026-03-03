#include <stdio.h>
#include "wifi_manage.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include "nvs.h"
#include "dns_server.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "lwip/inet.h"

// WiFi配置参数
#define WIFI_AP_SSID "ESP32_Config" // 配网热点名称
#define WIFI_AP_PASS "12345678"     // 配网热点密码
#define WIFI_AP_CHANNEL 1           // WiFi信道
#define WIFI_CONNECT_TIMEOUT 30000  // 连接超时时间(ms)
#define WIFI_AP_MAX_CONN 1          // 最大连接数

static const char *TAG = "WIFI_MANAGE";
static httpd_handle_t server = NULL;            // HTTP服务器句柄
static wifi_manage_cb_t status_callback = NULL; // WiFi状态回调函数
static dns_server_handle_t dns_handle = NULL;

// 嵌入HTML文件到固件
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

// 添加一个全局变量来存储最后的连接状态
static struct
{
    bool connecting;
    char message[128];
    bool connected;
    char ssid[33];
    char ip[16];
} wifi_state = {
    .connecting = false,
    .message = "",
    .connected = false,
    .ssid = "",
    .ip = ""};

// 添加一个静态变量来控制是否需要自动重连
static bool auto_reconnect = false;

// ========== 其余逻辑（DNS 启动与 WiFi 事件） ==========
static void start_dns_redirect(void)
{
    // get the IP of the access point to redirect to
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

    char ip_addr[16];
    inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
    ESP_LOGI("AP_IP", "Set up softAP with IP: %s", ip_addr);

    // turn the IP into a URI
    char *captiveportal_uri = (char *)malloc(32 * sizeof(char));
    assert(captiveportal_uri && "Failed to allocate captiveportal_uri");
    strcpy(captiveportal_uri, "http://");
    strcat(captiveportal_uri, ip_addr);

    // get a handle to configure DHCP with
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

    // set the DHCP option 114
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(netif));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(netif, ESP_NETIF_OP_SET, ESP_NETIF_CAPTIVEPORTAL_URI, captiveportal_uri, strlen(captiveportal_uri)));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(netif));
    dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE("*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
    dns_handle = start_dns_server(&config);
    free(captiveportal_uri);
}

/**
 * @brief 保存WiFi配置到NVS
 *
 * @param ssid WiFi名称
 * @param password WiFi密码
 * @return esp_err_t
 */
static esp_err_t save_wifi_config(const char *ssid, const char *password)
{

    nvs_handle_t nvs_handle;
    esp_err_t ret;

    // 打开NVS命名空间
    ret = nvs_open("wifi_config", NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK)
        return ret;

    // 保存SSID
    ret = nvs_set_str(nvs_handle, "ssid", ssid);
    if (ret != ESP_OK)
    {
        nvs_close(nvs_handle);
        return ret;
    }

    // 保存密码
    ret = nvs_set_str(nvs_handle, "password", password);
    if (ret != ESP_OK)
    {
        nvs_close(nvs_handle);
        return ret;
    }

    // 提交更改
    ret = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    ESP_LOGI(TAG, "保存WiFi配置 - SSID: %s, Password: %s", ssid, password);
    return ret;
}

/**
 * @brief 读取WiFi配置
 *
 * @param ssid
 * @param password
 * @return esp_err_t
 */
static esp_err_t read_wifi_config(char *ssid, char *password)
{
    nvs_handle_t nvs_handle;
    esp_err_t ret;
    size_t ssid_len;
    size_t pass_len;

    // 打开nvs
    ret = nvs_open("wifi_config", NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open nvs");
        return ret;
    }

    // 先获取实际长度
    ret = nvs_get_str(nvs_handle, "ssid", NULL, &ssid_len);
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to get ssid length");
        nvs_close(nvs_handle);
        return ret;
    }

    // 读取ssid
    ret = nvs_get_str(nvs_handle, "ssid", ssid, &ssid_len);
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to read ssid");
        nvs_close(nvs_handle);
        return ret;
    }

    // 先获取密码实际长度
    ret = nvs_get_str(nvs_handle, "password", NULL, &pass_len);
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to get password length");
        nvs_close(nvs_handle);
        return ret;
    }

    // 读取密码
    ret = nvs_get_str(nvs_handle, "password", password, &pass_len);
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "Failed to read password");
        nvs_close(nvs_handle);
        return ret;
    }

    // 关闭nvs
    nvs_close(nvs_handle);
    ESP_LOGI(TAG, "Read WiFi config success - SSID: %s", ssid);
    return ESP_OK;
}

/**
 * @brief 处理根路径请求，返回配网页面
 */
static esp_err_t root_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

/**
 * @brief 处理WiFi连接请求
 * 接收JSON格式：{"ssid": "xxx", "password": "xxx"}
 */
static esp_err_t connect_handler(httpd_req_t *req)
{
    char buf[128];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
        return ESP_FAIL;
    buf[ret] = '\0';

    // 解析JSON数据
    cJSON *root = cJSON_Parse(buf);
    if (!root)
        return ESP_FAIL;

    const char *ssid = cJSON_GetObjectItem(root, "ssid")->valuestring;
    const char *password = cJSON_GetObjectItem(root, "password")->valuestring;

    // 保存WiFi配置
    if (save_wifi_config(ssid, password) != ESP_OK)
    {
        cJSON_Delete(root);
        httpd_resp_send(req, "Failed to save configuration", -1);
        return ESP_FAIL;
    }

    // 配置WiFi连接参数
    wifi_config_t sta_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true, // 支持PMF
                .required = false},
        },
    };
    strlcpy((char *)sta_config.sta.ssid, ssid, sizeof(sta_config.sta.ssid));
    strlcpy((char *)sta_config.sta.password, password, sizeof(sta_config.sta.password));

    // 设置连接状态
    wifi_state.connecting = true;
    auto_reconnect = true; // 新的连接请求时启用自动重连
    snprintf(wifi_state.message, sizeof(wifi_state.message), "正在连接到 %s...", ssid);

    // 设置STA配置并连接
    if (esp_wifi_set_config(WIFI_IF_STA, &sta_config) != ESP_OK)
    {
        ESP_LOGW(TAG, "设置STA配置失败");
        cJSON_Delete(root);
        return ESP_FAIL;
    }
    if (esp_wifi_connect() != ESP_OK)
    {
        ESP_LOGW(TAG, "连接到STA失败");
        cJSON_Delete(root);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "设置STA配置成功");

    cJSON_Delete(root);
    httpd_resp_send(req, "Success! Device is connecting to WiFi...", -1);
    return ESP_OK;
}

// 处理WiFi扫描请求
static esp_err_t scan_handler(httpd_req_t *req)
{
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false};

    // 开始扫描
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

    // 获取扫描结果
    uint16_t ap_count = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

    wifi_ap_record_t *ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
    if (ap_records == NULL)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Out of memory");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_records));

    // 构建JSON响应
    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < ap_count; i++)
    {
        cJSON *ap = cJSON_CreateObject();
        cJSON_AddStringToObject(ap, "ssid", (char *)ap_records[i].ssid);
        cJSON_AddNumberToObject(ap, "rssi", ap_records[i].rssi);
        cJSON_AddBoolToObject(ap, "auth", ap_records[i].authmode != WIFI_AUTH_OPEN);
        cJSON_AddItemToArray(root, ap);
    }

    char *json_string = cJSON_Print(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_string, strlen(json_string));

    free(json_string);
    cJSON_Delete(root);
    free(ap_records);
    return ESP_OK;
}

// 添加一个状态查询处理函数
static esp_err_t status_handler(httpd_req_t *req)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "connecting", wifi_state.connecting);
    cJSON_AddBoolToObject(root, "connected", wifi_state.connected);
    cJSON_AddStringToObject(root, "message", wifi_state.message);
    cJSON_AddStringToObject(root, "ssid", wifi_state.ssid);
    cJSON_AddStringToObject(root, "ip", wifi_state.ip);

    char *json_string = cJSON_Print(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_string, strlen(json_string));

    free(json_string);
    cJSON_Delete(root);
    return ESP_OK;
}

// 添加断开连接处理函数
static esp_err_t disconnect_handler(httpd_req_t *req)
{
    auto_reconnect = false; // 手动断开时禁用自动重连
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    httpd_resp_send(req, "Disconnected", -1);
    return ESP_OK;
}

// 添加断开连接的URI处理
static const httpd_uri_t disconnect = {
    .uri = "/disconnect",
    .method = HTTP_POST,
    .handler = disconnect_handler,
    .user_ctx = NULL};

// HTTP服务器URI处理配置
static const httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_handler,
    .user_ctx = NULL};

static const httpd_uri_t connect = {
    .uri = "/connect",
    .method = HTTP_POST,
    .handler = connect_handler,
    .user_ctx = NULL};

static const httpd_uri_t scan = {
    .uri = "/scan",
    .method = HTTP_GET,
    .handler = scan_handler,
    .user_ctx = NULL};

static const httpd_uri_t status = {
    .uri = "/status",
    .method = HTTP_GET,
    .handler = status_handler,
    .user_ctx = NULL};
static const httpd_uri_t genera_204 = {
    .uri = "/*",
    .method = HTTP_ANY,
    .handler = root_handler,
    .user_ctx = NULL};

/**
 * @brief WiFi事件处理函数
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_AP_START: // 配网热点启动
            if (status_callback)
                status_callback(WIFI_MANAGE_EVENT_AP_START);
            break;
        case WIFI_EVENT_AP_STOP: // 配网热点停止
            if (status_callback)
                status_callback(WIFI_MANAGE_EVENT_AP_STOP);
            break;
        case WIFI_EVENT_STA_START: // 连接到STA
            if (status_callback)
                status_callback(WIFI_MANAGE_EVENT_STA_START);
            break;
        case WIFI_EVENT_STA_CONNECTED: // 连接到STA成功
        {
            wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
            memcpy(wifi_state.ssid, event->ssid, event->ssid_len);
            wifi_state.ssid[event->ssid_len] = '\0';
            wifi_state.connected = true;
            auto_reconnect = true; // 连接成功后启用自动重连
            if (status_callback)
                status_callback(WIFI_MANAGE_EVENT_STA_CONNECTED);
            char temp[64];
            snprintf(temp, sizeof(temp), "%s", wifi_state.ssid);
            snprintf(wifi_state.message, sizeof(wifi_state.message),
                     "已连接到WiFi: %s", temp);
            ESP_LOGI(TAG, "已连接到WiFi: %s", temp);

            break;
        }
        case WIFI_EVENT_STA_DISCONNECTED: // 与目标WiFi的连接断开
        {
            wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
            wifi_state.connected = false;
            wifi_state.ssid[0] = '\0';
            wifi_state.ip[0] = '\0';
            if (status_callback)
                status_callback(WIFI_MANAGE_EVENT_STA_DISCONNECTED);
            wifi_state.connecting = false;

            // 添加更详细的错误信息
            const char *reason;
            switch (event->reason)
            {
            case WIFI_REASON_AUTH_EXPIRE:
                reason = "认证过期";
                break;
            case WIFI_REASON_AUTH_FAIL:
                reason = "认证失败";
                break;
            case WIFI_REASON_NO_AP_FOUND:
                reason = "未找到AP";
                break;
            case WIFI_REASON_ASSOC_FAIL:
                reason = "关联失败";
                break;
            case WIFI_REASON_HANDSHAKE_TIMEOUT:
                reason = "握手超时";
                break;
            default:
                reason = "未知原因";
                break;
            }
            snprintf(wifi_state.message, sizeof(wifi_state.message),
                     "WiFi断开连接: %s", reason);
            ESP_LOGW(TAG, "WiFi断开连接，原因: %s (code: %d)", reason, event->reason);

            // 如果启用了自动重连，则尝试重新连接
            if (auto_reconnect)
            {
                ESP_LOGI(TAG, "尝试重新连接...");
                esp_wifi_connect();
            }
            break;
        }
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) // 获取IP地址
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        snprintf(wifi_state.ip, sizeof(wifi_state.ip), IPSTR, IP2STR(&event->ip_info.ip));
        if (status_callback)
            status_callback(WIFI_MANAGE_EVENT_STA_GOT_IP);
        wifi_state.connecting = false;
        char temp[96];
        snprintf(temp, sizeof(temp), "WiFi: %s", wifi_state.ssid);
        snprintf(wifi_state.message, sizeof(wifi_state.message),
                 "%s\nIP: %s", temp, wifi_state.ip);
    }
}

/**
 * @brief 启动HTTP服务器
 */
static esp_err_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // 修改HTTP服务器配置
    config.uri_match_fn = httpd_uri_match_wildcard; // 使用通配符匹配URI
    config.max_uri_handlers = 20;                   // 最大URI处理数
    config.max_resp_headers = 8;                    // 最大响应头数
    config.recv_wait_timeout = 30;                  // 接收等待超时时间
    config.send_wait_timeout = 30;                  // 发送等待超时时间
    config.lru_purge_enable = true;                 // 启用LRU清除
    config.max_req_hdr_len = 2048;
    config.max_uri_len = 512;

    // 启动HTTP服务器
    if (httpd_start(&server, &config) != ESP_OK)
    {
        return ESP_FAIL;
    }

    // 注册URI处理函数
    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &connect);
    httpd_register_uri_handler(server, &scan);
    httpd_register_uri_handler(server, &status);
    httpd_register_uri_handler(server, &disconnect);
    httpd_register_uri_handler(server, &genera_204);
    return ESP_OK;
}

esp_err_t wifi_manage_init(wifi_manage_cb_t cb)
{
    status_callback = cb;

    // 初始化网络接口
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 创建默认的AP和STA网络接口
    esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    // 初始化WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册WiFi事件处理函数
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler, NULL));

    return ESP_OK;
}

esp_err_t wifi_manage_start(void)
{
    char sta_ssid[32];
    char sta_password[64];

    // 设置为AP+STA模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    // 配置AP模式参数
    wifi_config_t ap_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .password = WIFI_AP_PASS,
            .channel = WIFI_AP_CHANNEL,
            .max_connection = WIFI_AP_MAX_CONN,
            .authmode = WIFI_AUTH_OPEN},
    };

    // 设置AP配置
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

    // 启动WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    // 添加短暂延时，等待WiFi完全初始化
    vTaskDelay(pdMS_TO_TICKS(500));

    // 读取WiFi配置并尝试连接
    if (read_wifi_config(sta_ssid, sta_password) == ESP_OK)
    {
        ESP_LOGI(TAG, "WiFi配置已读取: %s, %s", sta_ssid, sta_password);
        // 配置STA配置
        wifi_config_t sta_config = {0}; // 先清零整个结构体

        // 复制SSID和密码
        memcpy(sta_config.sta.ssid, sta_ssid, strlen(sta_ssid));
        memcpy(sta_config.sta.password, sta_password, strlen(sta_password));

        // 设置认证模式
        sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        sta_config.sta.pmf_cfg.capable = true;
        sta_config.sta.pmf_cfg.required = false;

        ESP_LOGI(TAG, "正在连接到WiFi: %s", sta_config.sta.ssid);

        // 设置STA配置
        if (esp_wifi_set_config(WIFI_IF_STA, &sta_config) != ESP_OK)
        {
            ESP_LOGE(TAG, "设置STA配置失败");
            return ESP_FAIL;
        }

        // // 启用自动重连
        auto_reconnect = true;

        // 连接到STA
        if (esp_wifi_connect() != ESP_OK)
        {
            ESP_LOGE(TAG, "连接到STA失败");
            return ESP_FAIL;
        }
    }

    // 启动Web服务器
    start_dns_redirect();
    ESP_ERROR_CHECK(start_webserver());

    ESP_LOGI(TAG, "WiFi配网已启动，请连接到 %s 进行配置", WIFI_AP_SSID);
    return ESP_OK;
}

// 实现断开连接接口
esp_err_t wifi_manage_disconnect(void)
{
    return esp_wifi_disconnect();
}

// 获取WiFi连接状态
uint8_t wifi_manage_get_status(void)
{
    if (wifi_state.connected == true && wifi_state.ip[0] != '\0')
    {
        return 1;
    }
    return 0; // 未连接
}

// 获取设备MAC地址
esp_err_t wifi_manage_get_mac(uint8_t *mac)
{
    esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, mac);

    return ret;
}

/**
 * @brief 停止AP模式，保留STA模式继续工作
 * @return esp_err_t 错误码，ESP_OK表示成功
 */
esp_err_t stop_ap_keep_sta(void)
{
    esp_err_t ret = ESP_OK;

    // 1. 获取当前WiFi模式，确认是AP+STA双模
    wifi_mode_t current_mode;
    ret = esp_wifi_get_mode(&current_mode);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get WiFi mode: %s", esp_err_to_name(ret));
        return ret;
    }
    if (current_mode != WIFI_MODE_APSTA)
    {
        ESP_LOGW(TAG, "WiFi is not in AP+STA mode, skip stop AP");
        return ESP_OK;
    }

    // 2. 停止AP的DHCP服务器（AP模式下DHCP服务器默认开启）
    esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (ap_netif)
    {
        // 停止DHCP服务器，避免资源泄漏
        ret = esp_netif_dhcps_stop(ap_netif);
        if (ret != ESP_OK && ret != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STOPPED)
        {
            ESP_LOGE(TAG, "Failed to stop AP DHCP server: %s", esp_err_to_name(ret));
            return ret;
        }
        ESP_LOGI(TAG, "AP DHCP server stopped");
    }

    // 3. 将WiFi模式改为仅STA模式，这会关闭AP但保留STA
    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set WiFi mode to STA: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "AP mode stopped, STA mode is kept");

    return ESP_OK;
}

/**
 * @brief 恢复AP模式（STA模式保持不变）
 * @return esp_err_t 错误码，ESP_OK表示成功
 */
esp_err_t resume_ap_keep_sta(void)
{
    esp_err_t ret = ESP_OK;

    // 1. 将WiFi模式改回AP+STA双模
    ret = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set WiFi mode to APSTA: %s", esp_err_to_name(ret));
        return ret;
    }

    // 2. 重启AP的DHCP服务器
    esp_netif_t *ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (ap_netif)
    {
        ret = esp_netif_dhcps_start(ap_netif);
        if (ret != ESP_OK && ret != ESP_ERR_ESP_NETIF_DHCP_ALREADY_STARTED)
        {
            ESP_LOGE(TAG, "Failed to start AP DHCP server: %s", esp_err_to_name(ret));
            return ret;
        }
        ESP_LOGI(TAG, "AP DHCP server started");
    }

    ESP_LOGI(TAG, "AP mode resumed, STA mode is kept");
    return ESP_OK;
}