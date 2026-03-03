// main.c - ESP32-CAM MJPEG 流 + 美观网页 + 拍照功能（2026 优化版）

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi.h" // 你的 WiFi 连接代码
#include "cam.h"  // 你的摄像头初始化代码（init_camera）
#include "sd.h"
static const char *TAG = "ESP32-CAM";

// ────────────────────────────────────────────────
//          MJPEG 流相关定义（更现代风格）
// ────────────────────────────────────────────────
#define PART_BOUNDARY "----ESP32CAMBoundary2026"
static const char *STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART_HEADER = "Content-Type: image/jpeg\r\n"
                                        "Content-Length: %u\r\n"
                                        "X-Timestamp: %ld.%06ld\r\n\r\n";

// ────────────────────────────────────────────────
//          美观的 HTML 界面（保持原样，仅微调）
// ────────────────────────────────────────────────
static const char *index_html =
    "<!DOCTYPE html><html lang=\"zh-CN\"><head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>ESP32-CAM 实时监控</title>"
    "<style>"
    "body{margin:0;padding:0;background:linear-gradient(135deg,#1e3c72 0%,#2a5298 100%);"
    "font-family:'Segoe UI',Arial,sans-serif;color:#fff;min-height:100vh;}"
    ".container{max-width:960px;margin:24px auto;padding:24px;background:rgba(255,255,255,0.08);"
    "border-radius:20px;box-shadow:0 12px 40px rgba(0,0,0,0.5);backdrop-filter:blur(12px);}"
    ".header{text-align:center;font-size:2.4rem;font-weight:700;margin-bottom:1.5rem;}"
    ".video-wrapper{background:#000;border-radius:16px;overflow:hidden;"
    "box-shadow:0 10px 30px rgba(0,0,0,0.6);position:relative;margin:1.5rem 0;}"
    "img{width:100%;height:auto;display:block;}"
    ".status{position:absolute;bottom:12px;left:12px;background:rgba(0,0,0,0.65);"
    "padding:6px 14px;border-radius:8px;font-size:0.9rem;}"
    ".info{text-align:center;font-size:1.1rem;line-height:1.6;}"
    ".info a{color:#4fc3f7;text-decoration:none;font-weight:600;transition:0.3s;}"
    ".info a:hover{color:#81d4fa;}"
    "@media (max-width:768px){.container{margin:16px;padding:16px;}"
    ".header{font-size:2rem;}}"
    "</style></head><body>"
    "<div class=\"container\">"
    "  <div class=\"header\">ESP32-CAM 实时视频监控</div>"
    "  <div class=\"video-wrapper\">"
    "    <img id=\"stream\" src=\"/stream\" alt=\"视频流加载中...\">"
    "    <div class=\"status\">SVGA 800×600 | 高画质 | 实时流</div>"
    "  </div>"
    "  <div class=\"info\">"
    "    <p>当前设置：SVGA，分辨率 800×600，jpeg_quality=12</p>"
    "    <p><a href=\"/capture\" target=\"_blank\">📸 拍摄单张照片</a></p>"
    "    <p>画面卡顿时可尝试降低分辨率或画质（需修改代码重新编译烧录）</p>"
    "  </div>"
    "</div>"
    "<script>"
    "const img = document.getElementById('stream');"
    "img.onerror = () => { setTimeout(() => { img.src = '/stream?' + Date.now(); }, 2000); };"
    "</script>"
    "</body></html>";

// ────────────────────────────────────────────────
//          处理器：主页
// ────────────────────────────────────────────────
static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    return httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
}

// ────────────────────────────────────────────────
//          处理器：单张拍照
// ────────────────────────────────────────────────
static esp_err_t capture_handler(httpd_req_t *req)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Camera capture failed");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=\"capture.jpg\"");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");

    esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);

    esp_camera_fb_return(fb);
    return res;
}

// ────────────────────────────────────────────────
//          处理器：MJPEG 视频流（带时间戳 + 更健壮）
// ────────────────────────────────────────────────
static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    char part_buf[160];
    struct timeval timestamp;

    // 响应头
    res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
        return res;

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "30"); // 提示性质，不强制

    while (res == ESP_OK)
    {
        fb = esp_camera_fb_get();
        if (!fb)
        {
            ESP_LOGE(TAG, "Camera capture failed in stream");
            res = ESP_FAIL;
            break;
        }

        gettimeofday(&timestamp, NULL);

        // 准备 part header
        size_t hlen = snprintf(part_buf, sizeof(part_buf), STREAM_PART_HEADER,
                               fb->len,
                               timestamp.tv_sec, timestamp.tv_usec);

        // 发送 boundary + header + jpeg 数据
        do
        {
            if (res == ESP_OK)
                res = httpd_resp_send_chunk(req, STREAM_BOUNDARY, HTTPD_RESP_USE_STRLEN);

            if (res == ESP_OK)
                res = httpd_resp_send_chunk(req, part_buf, hlen);

            if (res == ESP_OK)
                res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        } while (0);

        esp_camera_fb_return(fb);

        // 防止 CPU 100% 占用，也避免过快导致网络拥塞
        vTaskDelay(pdMS_TO_TICKS(33)); // ≈30fps
    }

    ESP_LOGI(TAG, "Stream connection closed");
    return res;
}

// ────────────────────────────────────────────────
//          启动 Web 服务器
// ────────────────────────────────────────────────
static esp_err_t start_web_server(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // 可根据内存情况微调
    config.server_port = 80;
    config.max_open_sockets = 7;
    config.lru_purge_enable = true;
    config.stack_size = 8192; // 加大一点更稳定

    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(ret));
        return ret;
    }

    // 注册 URI
    httpd_uri_t uris[] = {
        {.uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL},
        {.uri = "/stream", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL},
        {.uri = "/capture", .method = HTTP_GET, .handler = capture_handler, .user_ctx = NULL},
    };

    for (size_t i = 0; i < sizeof(uris) / sizeof(uris[0]); i++)
    {
        ret = httpd_register_uri_handler(server, &uris[i]);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to register %s handler", uris[i].uri);
            httpd_stop(server);
            return ret;
        }
    }

    ESP_LOGI(TAG, "Web server started successfully on port 80");
    return ESP_OK;
}

// ────────────────────────────────────────────────
//          主函数
// ────────────────────────────────────────────────
void app_main(void)
{
    // 初始化 NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 连接 Wi-Fi
    wifi_start(); // 请确保此函数会阻塞直到连上或超时

    // 初始化摄像头
    if (init_camera() != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera init failed → program stopped");
        return;
    }

    // 可选：传感器微调
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor)
    {
        sensor->set_hmirror(sensor, 1); // 水平翻转（常用）
        // sensor->set_vflip(sensor, 1);    // 垂直翻转（按需打开）
        sensor->set_quality(sensor, 12); // 10~63，数字越小质量越高
        // sensor->set_framesize(sensor, FRAMESIZE_SVGA);

        ESP_LOGI(TAG, "Sensor configured: hmirror=1, quality=12");
    }

    // 等待摄像头稳定
    vTaskDelay(pdMS_TO_TICKS(2500));

    // 启动 Web 服务器
    if (start_web_server() != ESP_OK)
    {
        ESP_LOGE(TAG, "Web server start failed");
    }

    vTaskDelay(pdMS_TO_TICKS(3000));
    sd_init();
    sd_save_jpg(1);
    // sd_write_jpg(fb->buf, fb->len);
    // esp_camera_fb_return(fb);
    // // sd_read_file("/sdcard/0001.jpg");
    // const char *jpg_path = "/sdcard/0001.jpg"; // SD卡上的JPG文件路径
    // uint8_t *jpg_data = NULL;
    // size_t jpg_size = 0;

    // esp_err_t reta = sd_read_jpg_file(jpg_path, &jpg_data, &jpg_size);
    // if (reta == ESP_OK)
    // {
    //     // 这里可以处理图片数据（如显示、传输、保存等）
    //     ESP_LOGI(TAG, "JPG data ready, size: %zu bytes", jpg_size);
    //     ESP_LOG_BUFFER_HEX(TAG, jpg_data, jpg_size);
    //     // 用完后必须释放内存
    //     free(jpg_data);
    //     jpg_data = NULL;
    // }
    // else
    // {
    //     ESP_LOGE(TAG, "Failed to read JPG file, err: %d", ret);
    // }
    // 启动任务
    // 主循环（保持任务不退出）
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        // 可在此处添加状态打印、心跳等
    }
}