#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>
#include <time.h>

// SD 卡相關
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

// Base64
#include "mbedtls/base64.h"

#define AP_SSID "ESP32-CAM-AP"
#define AP_PASSWORD "12345678"
#define AP_CHANNEL 1
#define AP_MAX_CONN 4

#define MOUNT_POINT "/sdcard"

static const char *TAG = "CAM-SD";

// ── 攝像頭參數 ──
typedef struct
{
    framesize_t frame_size;
    uint8_t quality;
    int8_t brightness;
    int8_t contrast;
    int8_t saturation;
    int8_t special_effect;
    uint8_t hmirror;
    uint8_t vflip;
} camera_params_t;

static camera_params_t cam_params = {
    .frame_size = FRAMESIZE_QVGA,
    .quality = 12,
    .brightness = 0,
    .contrast = 0,
    .saturation = 0,
    .special_effect = 0,
    .hmirror = 0,
    .vflip = 0};

// ── 最常見的 AI-Thinker ESP32-CAM 腳位 ──
#define CAM_PIN_PWDN 38
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 15
#define CAM_PIN_SIOD 4
#define CAM_PIN_SIOC 5
#define CAM_PIN_D7 16
#define CAM_PIN_D6 17
#define CAM_PIN_D5 18
#define CAM_PIN_D4 12
#define CAM_PIN_D3 10
#define CAM_PIN_D2 8
#define CAM_PIN_D1 9
#define CAM_PIN_D0 11
#define CAM_PIN_VSYNC 6
#define CAM_PIN_HREF 7
#define CAM_PIN_PCLK 13

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,
    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_QVGA,
    .jpeg_quality = 12,
    .fb_count = 2,
    .grab_mode = CAMERA_GRAB_LATEST};

// ── HTML ──（按鈕文字已微調，更清楚說明功能）
static const char *html_page =
    "<!DOCTYPE html><html><head>"
    "<meta charset=\"UTF-8\"><title>ESP32-CAM 控制</title>"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "<style>"
    "body{font-family:Arial,sans-serif;margin:0;padding:15px;background:#f0f2f5;}"
    ".container{max-width:900px;margin:auto;background:white;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);}"
    ".header{background:#1a3c5e;color:white;padding:15px;text-align:center;font-size:1.4em;}"
    "img{width:100%;display:block;}"
    ".controls{padding:20px;}"
    ".group{margin:15px 0;}"
    "label{display:inline-block;width:160px;font-weight:bold;}"
    "select,input[type=range]{padding:8px;width:220px;}"
    ".buttons{margin:25px 0;display:flex;gap:15px;flex-wrap:wrap;}"
    "button{padding:12px 24px;font-size:1em;border:none;border-radius:6px;cursor:pointer;color:white;}"
    ".btn-apply{background:#007bff;}.btn-apply:hover{background:#0069d9;}"
    ".btn-save{background:#28a745;}.btn-save:hover{background:#218838;}"
    ".btn-print{background:#17a2b8;}.btn-print:hover{background:#138496;}"
    ".btn-base64{background:#6c757d;}.btn-base64:hover{background:#5a6268;}"
    "#msg{margin-top:20px;padding:10px;background:#e9ecef;border-radius:6px;min-height:50px;}"
    "</style>"
    "<script>"
    "function update(id,val){document.getElementById(id+'-val').innerText=val}"
    "async function saveToSD(){"
    "  document.getElementById('msg').innerText='正在拍照 → 存SD卡 + 輸出Base64到串口...';"
    "  try{"
    "    const r=await fetch('/save_sd');"
    "    const txt=await r.text();"
    "    document.getElementById('msg').innerText=txt;"
    "  }catch(e){document.getElementById('msg').innerText='錯誤：'+e;}"
    "}"
    "async function printBase64ToSerial(){"
    "  document.getElementById('msg').innerText='正在拍照並將 Base64 輸出到串口...';"
    "  try{"
    "    const r=await fetch('/print_base64');"
    "    const txt=await r.text();"
    "    document.getElementById('msg').innerText=txt;"
    "  }catch(e){document.getElementById('msg').innerText='錯誤：'+e;}"
    "}"
    "async function getBase64(){"
    "  document.getElementById('msg').innerText='正在取得 Base64...';"
    "  try{"
    "    const r=await fetch('/capture_base64');"
    "    const t=await r.text();"
    "    document.getElementById('base64').textContent=t;"
    "    document.getElementById('msg').innerText='Base64 已顯示在下方';"
    "  }catch(e){document.getElementById('msg').innerText='錯誤：'+e;}"
    "}"
    "</script>"
    "</head><body>"
    "<div class=\"container\">"
    "  <div class=\"header\">ESP32-CAM 即時影像與控制</div>"
    "  <img src=\"/stream\" alt=\"即時影像\">"
    "  <div class=\"controls\">"
    "    <form action=\"/set\" method=\"post\">"
    "      <div class=\"group\"><label>解析度：</label><select name=\"size\">"
    "        <option value=\"7\">QVGA (320×240)</option>"
    "        <option value=\"8\">VGA (640×480)</option>"
    "        <option value=\"9\">SVGA (800×600)</option>"
    "      </select></div>"
    "      <div class=\"group\"><label>畫質 (0~63)：</label>"
    "        <input type=\"range\" name=\"quality\" min=\"0\" max=\"63\" value=\"12\" oninput=\"update('quality',this.value)\">"
    "        <span id=\"quality-val\">12</span></div>"
    "      <div class=\"group\"><label>亮度 (-2~2)：</label>"
    "        <input type=\"range\" name=\"brightness\" min=\"-2\" max=\"2\" value=\"0\" oninput=\"update('brightness',this.value)\">"
    "        <span id=\"brightness-val\">0</span></div>"
    "      <div class=\"group\"><label>對比度 (-2~2)：</label>"
    "        <input type=\"range\" name=\"contrast\" min=\"-2\" max=\"2\" value=\"0\" oninput=\"update('contrast',this.value)\">"
    "        <span id=\"contrast-val\">0</span></div>"
    "      <div class=\"group\"><label>飽和度 (-2~2)：</label>"
    "        <input type=\"range\" name=\"saturation\" min=\"-2\" max=\"2\" value=\"0\" oninput=\"update('saturation',this.value)\">"
    "        <span id=\"saturation-val\">0</span></div>"
    "      <div class=\"buttons\"><button type=\"submit\" class=\"btn-apply\">套用設定</button></div>"
    "    </form>"
    "    <div class=\"buttons\">"
    "      <button onclick=\"saveToSD()\" class=\"btn-save\">拍照 → 存SD卡 + 印Base64到串口</button>"
    "      <button onclick=\"printBase64ToSerial()\" class=\"btn-print\">僅拍照並印 Base64 到串口</button>"
    "      <button onclick=\"getBase64()\" class=\"btn-base64\">顯示 Base64 在網頁</button>"
    "    </div>"
    "    <div id=\"msg\">點擊按鈕操作</div>"
    "    <pre id=\"base64\" style=\"margin-top:20px;padding:15px;background:#f8f9fa;border:1px solid #ddd;border-radius:6px;white-space:pre-wrap;word-break:break-all;max-height:300px;overflow-y:auto;font-size:13px;\">"
    "      Base64 顯示區..."
    "    </pre>"
    "  </div>"
    "</div>"
    "</body></html>";

// ── 手動設定 2026 年時間（開發/測試用） ──
static void set_system_time_2026(void)
{
    struct tm t = {
        .tm_year = 2026 - 1900,
        .tm_mon = 0,
        .tm_mday = 31,
        .tm_hour = 12,
        .tm_min = 0,
        .tm_sec = 0,
        .tm_isdst = -1};

    time_t now = mktime(&t);
    if (now == (time_t)-1)
    {
        ESP_LOGE(TAG, "mktime 轉換失敗");
        return;
    }

    struct timeval tv = {.tv_sec = now, .tv_usec = 0};
    settimeofday(&tv, NULL);

    ESP_LOGI(TAG, "系統時間已手動設定為 2026-01-31 12:00:00");
}

// ── SD 卡初始化 ──
static esp_err_t init_sd_card(void)
{
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;

    ESP_LOGI(TAG, "初始化 SD 卡 (SDMMC 1-bit 模式)");

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    slot_config.clk = GPIO_NUM_39;
    slot_config.cmd = GPIO_NUM_38;
    slot_config.d0 = GPIO_NUM_40;
    slot_config.width = 1;

    esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SD 卡掛載失敗 (%s)", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "SD 卡掛載成功");
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}

// ── 更新攝像頭參數 ──
void update_camera_params(void)
{
    sensor_t *s = esp_camera_sensor_get();
    if (s)
    {
        s->set_framesize(s, cam_params.frame_size);
        s->set_quality(s, cam_params.quality);
        s->set_brightness(s, cam_params.brightness);
        s->set_contrast(s, cam_params.contrast);
        s->set_saturation(s, cam_params.saturation);
        s->set_special_effect(s, cam_params.special_effect);
        s->set_hmirror(s, cam_params.hmirror);
        s->set_vflip(s, cam_params.vflip);
    }
}

// ── 共用拍照函式 ──
static camera_fb_t *take_photo(void)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGE(TAG, "Camera capture failed");
    }
    return fb;
}

// ── 核心功能：拍照 → 優先存 SD + 完整 Base64 印到串口 ──
static esp_err_t capture_and_save_sd_with_base64(void)
{
    camera_fb_t *fb = take_photo();
    if (!fb)
        return ESP_FAIL;

    esp_err_t sd_status = ESP_FAIL;

    // ── 1. 優先存到 SD 卡 ──
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char filename[64];
    snprintf(filename, sizeof(filename), "%s/photo_%04d%02d%02d_%02d%02d%02d.jpg",
             MOUNT_POINT,
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    FILE *f = fopen(filename, "wb");
    if (f)
    {
        fwrite(fb->buf, 1, fb->len, f);
        fclose(f);
        ESP_LOGI(TAG, "照片已儲存 → %s  (%u bytes)", filename, fb->len);
        sd_status = ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "無法寫入檔案：%s", filename);
    }

    // ── 2. 轉 Base64 並完整輸出到串口 ──
    size_t olen = 0;
    size_t elen = ((fb->len + 2) / 3 * 4) + 32;
    unsigned char *b64 = malloc(elen);
    if (b64)
    {
        if (mbedtls_base64_encode(b64, elen, &olen, fb->buf, fb->len) == 0)
        {
            ESP_LOGI(TAG, "=== JPEG Base64 START (jpeg size=%u, base64 len=%u) ===", fb->len, olen);
            ESP_LOG_BUFFER_CHAR(TAG, (const char *)b64, olen); // 完整連續輸出
            ESP_LOGI(TAG, "=== JPEG Base64 END ===");
        }
        else
        {
            ESP_LOGE(TAG, "Base64 編碼失敗");
        }
        free(b64);
    }
    else
    {
        ESP_LOGE(TAG, "Base64 記憶體不足");
    }

    esp_camera_fb_return(fb);
    return sd_status;
}

// ── 僅輸出 Base64 到串口（原本的 print_base64 功能） ──
static esp_err_t print_base64_to_serial(void)
{
    camera_fb_t *fb = take_photo();
    if (!fb)
        return ESP_FAIL;

    size_t olen = 0;
    size_t elen = ((fb->len + 2) / 3 * 4) + 32;
    unsigned char *b64 = malloc(elen);
    if (!b64)
    {
        esp_camera_fb_return(fb);
        return ESP_FAIL;
    }

    if (mbedtls_base64_encode(b64, elen, &olen, fb->buf, fb->len) == 0)
    {
        ESP_LOGI(TAG, "=== JPEG Base64 START (len=%u) ===", olen);
        ESP_LOG_BUFFER_CHAR(TAG, (const char *)b64, olen);
        ESP_LOGI(TAG, "=== JPEG Base64 END ===");
    }
    else
    {
        ESP_LOGE(TAG, "Base64 編碼失敗");
    }

    free(b64);
    esp_camera_fb_return(fb);
    return ESP_OK;
}

// ── HTTP Handlers ──
static esp_err_t root_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    return httpd_resp_send(req, html_page, strlen(html_page));
}

static esp_err_t set_handler(httpd_req_t *req)
{
    char buf[512] = {0};
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0)
        goto redirect;

    buf[ret] = '\0';
    char *p = buf, *token;

    while ((token = strtok_r(p, "&", &p)))
    {
        if (strstr(token, "size="))
            cam_params.frame_size = atoi(token + 5);
        if (strstr(token, "quality="))
            cam_params.quality = atoi(token + 8);
        if (strstr(token, "brightness="))
            cam_params.brightness = atoi(token + 11);
        if (strstr(token, "contrast="))
            cam_params.contrast = atoi(token + 9);
        if (strstr(token, "saturation="))
            cam_params.saturation = atoi(token + 11);
    }

    update_camera_params();

redirect:
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t stream_handler(httpd_req_t *req)
{
    char part_buf[64];
    strcpy(part_buf, "--frame\r\nContent-Type: image/jpeg\r\n\r\n");

    httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");

    while (true)
    {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            vTaskDelay(1);
            continue;
        }

        if (httpd_resp_send_chunk(req, part_buf, strlen(part_buf)) != ESP_OK ||
            httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len) != ESP_OK)
        {
            esp_camera_fb_return(fb);
            break;
        }
        esp_camera_fb_return(fb);
        vTaskDelay(pdMS_TO_TICKS(80));
    }
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t save_sd_handler(httpd_req_t *req)
{
    esp_err_t ret = capture_and_save_sd_with_base64();

    const char *msg = (ret == ESP_OK)
                          ? "照片已儲存到 SD 卡，並已將完整 Base64 輸出到串口"
                          : "SD 卡儲存失敗，但已嘗試將 Base64 輸出到串口（請檢查 SD 卡）";

    httpd_resp_send(req, msg, strlen(msg));
    return ESP_OK;
}

static esp_err_t print_base64_handler(httpd_req_t *req)
{
    esp_err_t ret = print_base64_to_serial();
    const char *msg = (ret == ESP_OK)
                          ? "Base64 已輸出到串口，請查看監控視窗"
                          : "拍照失敗，無法輸出 Base64";
    httpd_resp_send(req, msg, strlen(msg));
    return ESP_OK;
}

static esp_err_t capture_base64_handler(httpd_req_t *req)
{
    camera_fb_t *fb = take_photo();
    if (!fb)
        return httpd_resp_send_500(req);

    size_t olen = 0;
    size_t elen = ((fb->len + 2) / 3 * 4) + 32;
    unsigned char *out = malloc(elen);
    if (!out)
    {
        esp_camera_fb_return(fb);
        return httpd_resp_send_500(req);
    }

    mbedtls_base64_encode(out, elen, &olen, fb->buf, fb->len);
    esp_camera_fb_return(fb);

    httpd_resp_set_type(req, "text/plain");
    esp_err_t res = httpd_resp_send(req, (char *)out, olen);
    free(out);
    return res;
}

// ── 啟動 Web 伺服器 ──
static void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 20;
    config.stack_size = 8192;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t uris[] = {
            {"/", HTTP_GET, root_handler, NULL},
            {"/set", HTTP_POST, set_handler, NULL},
            {"/stream", HTTP_GET, stream_handler, NULL},
            {"/save_sd", HTTP_GET, save_sd_handler, NULL},
            {"/print_base64", HTTP_GET, print_base64_handler, NULL},
            {"/capture_base64", HTTP_GET, capture_base64_handler, NULL},
        };

        for (size_t i = 0; i < sizeof(uris) / sizeof(uris[0]); i++)
        {
            httpd_register_uri_handler(server, &uris[i]);
        }

        ESP_LOGI(TAG, "HTTP Server started → http://192.168.4.1");
    }
    else
    {
        ESP_LOGE(TAG, "HTTP Server start failed");
    }
}

static void wifi_init_softap(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .channel = AP_CHANNEL,
            .password = AP_PASSWORD,
            .max_connection = AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    if (strlen(AP_PASSWORD) == 0)
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "AP started → SSID: %s  Password: %s", AP_SSID, AP_PASSWORD);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    set_system_time_2026();

    if (esp_camera_init(&camera_config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed");
        return;
    }
    update_camera_params();

    init_sd_card(); // 不強制失敗停止

    wifi_init_softap();
    start_webserver();
}