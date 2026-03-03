#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nocodec_audio.h" // 需包含 init_speaker() 和 audio_write()
#include "esp_audio_dec.h"
#include "esp_audio_dec_default.h"

// 嵌入的 MP3 数据（由 CMake 或 menuconfig 生成的符号）
extern const char succ_mp3_start[] asm("_binary_succ_mp3_start");
extern const char succ_mp3_end[] asm("_binary_succ_mp3_end");

#define AUDIO_DATA_START succ_mp3_start
#define AUDIO_DATA_SIZE (succ_mp3_end - succ_mp3_start)

#define TAG "MP3播放器"

// I2S 引脚（根据你的硬件板子修改）
#define I2S_BCLK GPIO_NUM_46
#define I2S_WS GPIO_NUM_9
#define I2S_SDOUT GPIO_NUM_8

// 音频参数（如果你的 succ.mp3 不是这个参数，请修改）
#define TARGET_SAMPLE_RATE 50000
#define TARGET_BITS_PER_SAMPLE 16
#define TARGET_CHANNELS 1

// 缓冲区大小建议值
#define INPUT_BUF_SIZE 4608      // 建议 ≥ 4× MP3 最大帧大小
#define OUTPUT_BUF_INIT_SZ 32768 // 初始 PCM 输出缓冲，会动态扩容

// 每次向 I2S 写入的样本数量（平衡延迟与效率）
#define SAMPLES_PER_WRITE 1152

static const char *err_to_str(esp_err_t err)
{
    switch (err)
    {
    case ESP_AUDIO_ERR_OK:
        return "成功";
    case ESP_AUDIO_ERR_FAIL:
        return "通用失败";
    case ESP_AUDIO_ERR_MEM_LACK:
        return "内存不足";
    case ESP_ERR_INVALID_ARG:
        return "参数错误";
    case ESP_AUDIO_ERR_BUFF_NOT_ENOUGH:
        return "输出缓冲不足";
    case ESP_AUDIO_ERR_NOT_SUPPORT:
        return "格式不支持";
    case ESP_AUDIO_ERR_NOT_FOUND:
        return "没有更多数据";
    case ESP_AUDIO_ERR_HEADER_PARSE:
        return "解码失败";
    default:
        return "未知错误";
    }
}

// 读取上下文
typedef struct
{
    const uint8_t *data; // 音频数据起始地址
    size_t total_size;   // 总长度
    size_t read_offset;  // 当前读取位置
    bool skip_id3;       // 是否需要跳过 ID3
    size_t id3_size;     // ID3 标签长度
} mp3_read_ctx_t;

static mp3_read_ctx_t g_read_ctx = {0};

// 解析 ID3v2 大小（同步安全整数）
static size_t parse_id3v2_size(const uint8_t *ptr)
{
    return ((ptr[0] & 0x7F) << 21) |
           ((ptr[1] & 0x7F) << 14) |
           ((ptr[2] & 0x7F) << 7) |
           (ptr[3] & 0x7F);
}

// 尝试跳过 ID3v2 标签，返回总长度（包含 10 字节头部）
static size_t skip_id3v2(const uint8_t *buf, size_t len)
{
    if (len < 10)
        return 0;

    if (buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3')
    {
        uint8_t version = buf[3];
        uint8_t flags = buf[5];
        size_t tag_sz = parse_id3v2_size(buf + 6);
        size_t total = 10 + tag_sz;

        ESP_LOGI(TAG, "发现 ID3v2 标签 v%d.%d，大小 %u 字节",
                 version >> 4, version & 0x0F, (unsigned)tag_sz);

        if (flags & 0x40)
        { // 有扩展头部
            if (len >= total + 4)
            {
                size_t ext_sz = (buf[10] << 24) | (buf[11] << 16) | (buf[12] << 8) | buf[13];
                total += ext_sz;
                ESP_LOGI(TAG, "扩展头部大小：%u 字节", (unsigned)ext_sz);
            }
        }
        return total;
    }
    return 0;
}

// 简单 MP3 帧同步字搜索
static int find_next_frame_sync(const uint8_t *buf, size_t len, size_t start)
{
    for (size_t i = start; i + 3 < len; i++)
    {
        if (buf[i] == 0xFF && (buf[i + 1] & 0xE0) == 0xE0)
        {
            uint8_t br_idx = (buf[i + 1] >> 4) & 0x0F;
            uint8_t sr_idx = (buf[i + 1] >> 2) & 0x03;
            if (br_idx != 0x0F && br_idx != 0 && sr_idx != 0x03)
            {
                return (int)i;
            }
        }
    }
    return -1;
}

// 解码器读取 MP3 数据的回调
static int mp3_read(uint8_t *dst, int want)
{
    if (want <= 0)
        return 0;

    size_t remain = g_read_ctx.total_size - g_read_ctx.read_offset;
    if (remain == 0)
        return 0;

    // 还在跳过 ID3 阶段
    if (g_read_ctx.skip_id3 && g_read_ctx.read_offset < g_read_ctx.id3_size)
    {
        size_t skip_left = g_read_ctx.id3_size - g_read_ctx.read_offset;
        size_t skip_now = (skip_left < (size_t)want) ? skip_left : (size_t)want;
        g_read_ctx.read_offset += skip_now;
        return 0;
    }

    size_t copy_sz = (remain < (size_t)want) ? remain : (size_t)want;
    memcpy(dst, g_read_ctx.data + g_read_ctx.read_offset, copy_sz);
    g_read_ctx.read_offset += copy_sz;

    return (int)copy_sz;
}

// PCM 数据写入 I2S
static int pcm_write(const uint8_t *pcm_data, int len_bytes)
{
    if (len_bytes <= 0)
        return 0;

    int samples = len_bytes / sizeof(int16_t);
    int written = audio_write((const int16_t *)pcm_data, samples);
    if (written < 0)
    {
        ESP_LOGE(TAG, "audio_write 失败: %d", written);
        return written;
    }
    return written * sizeof(int16_t);
}

void app_main(void)
{
    ESP_LOGI(TAG, "MP3 播放器启动 | 嵌入数据大小: %u 字节", (unsigned)AUDIO_DATA_SIZE);
    if (AUDIO_DATA_SIZE < 1024)
    {
        ESP_LOGE(TAG, "音频数据过小，无法播放！");
        goto inf_loop;
    }

    // 1. 跳过 ID3v2
    size_t id3_len = skip_id3v2((const uint8_t *)AUDIO_DATA_START, AUDIO_DATA_SIZE);
    g_read_ctx.data = (const uint8_t *)AUDIO_DATA_START;
    g_read_ctx.total_size = AUDIO_DATA_SIZE;
    g_read_ctx.read_offset = 0;
    g_read_ctx.id3_size = id3_len;
    g_read_ctx.skip_id3 = (id3_len > 0);

    if (id3_len > 0)
    {
        ESP_LOGI(TAG, "将跳过 %u 字节的 ID3v2 标签", (unsigned)id3_len);
    }

    // 2. 初始化 I2S
    init_speaker(I2S_BCLK, I2S_WS, I2S_SDOUT, TARGET_SAMPLE_RATE);
    ESP_LOGI(TAG, "I2S 初始化完成，采样率 %d Hz", TARGET_SAMPLE_RATE);

    // 3. 分配缓冲区
    uint8_t *in_buf = malloc(INPUT_BUF_SIZE);
    uint8_t *out_buf = malloc(OUTPUT_BUF_INIT_SZ);
    if (!in_buf || !out_buf)
    {
        ESP_LOGE(TAG, "内存分配失败");
        goto cleanup;
    }

    // 4. 注册默认解码器（包含 MP3）
    ESP_ERROR_CHECK(esp_audio_dec_register_default());

    // 5. 打开 MP3 解码器
    esp_audio_dec_handle_t decoder = NULL;
    esp_audio_dec_cfg_t cfg = {
        .type = ESP_AUDIO_TYPE_MP3,
        .cfg = NULL,
        .cfg_sz = 0,
    };

    esp_err_t ret = esp_audio_dec_open(&cfg, &decoder);
    if (ret != ESP_AUDIO_ERR_OK)
    {
        ESP_LOGE(TAG, "打开 MP3 解码器失败: %s (%d)", err_to_str(ret), ret);
        goto cleanup;
    }
    ESP_LOGI(TAG, "MP3 解码器打开成功");

    uint64_t start_time = esp_timer_get_time();
    uint64_t total_pcm = 0;
    uint32_t frame_count = 0;
    int error_count = 0;

    // 主解码循环
    while (1)
    {
        int read_len = mp3_read(in_buf, INPUT_BUF_SIZE);
        if (read_len <= 0)
        {
            if (g_read_ctx.read_offset >= g_read_ctx.total_size)
            {
                ESP_LOGI(TAG, "已读取完所有 MP3 数据");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(5));
            continue;
        }

        esp_audio_dec_in_raw_t raw_in = {
            .buffer = in_buf,
            .len = read_len,
            .consumed = 0};

        esp_audio_dec_out_frame_t out_frame = {
            .buffer = out_buf,
            .len = OUTPUT_BUF_INIT_SZ,
            .decoded_size = 0,
            .needed_size = 0};

        while (raw_in.len > 0)
        {
            ret = esp_audio_dec_process(decoder, &raw_in, &out_frame);

            if (ret == ESP_AUDIO_ERR_BUFF_NOT_ENOUGH)
            {
                size_t need = out_frame.needed_size;
                if (need > 2 * 1024 * 1024)
                {
                    ESP_LOGE(TAG, "所需输出缓冲过大：%u 字节", (unsigned)need);
                    goto cleanup;
                }
                uint8_t *new_buf = realloc(out_buf, need);
                if (!new_buf)
                {
                    ESP_LOGE(TAG, "realloc 失败：%u 字节", (unsigned)need);
                    goto cleanup;
                }
                out_buf = new_buf;
                out_frame.buffer = new_buf;
                out_frame.len = need;
                ESP_LOGW(TAG, "输出缓冲扩容至 %u 字节", (unsigned)need);
                continue;
            }

            if (ret != ESP_AUDIO_ERR_OK)
            {
                if (ret == ESP_AUDIO_ERR_NOT_FOUND)
                {
                    break;
                }

                error_count++;
                ESP_LOGE(TAG, "解码失败: %s (%d)  已消耗:%d  剩余:%d",
                         err_to_str(ret), ret, raw_in.consumed, raw_in.len);

                if (raw_in.consumed == 0 && raw_in.len > 4)
                {
                    int next = find_next_frame_sync(raw_in.buffer, raw_in.len, 1);
                    if (next > 0)
                    {
                        ESP_LOGW(TAG, "跳过 %d 字节到下一帧同步", next);
                        raw_in.buffer += next;
                        raw_in.len -= next;
                        g_read_ctx.read_offset += next;
                    }
                    else
                    {
                        raw_in.buffer++;
                        raw_in.len--;
                        g_read_ctx.read_offset++;
                    }
                }
                else if (raw_in.consumed > 0)
                {
                    raw_in.buffer += raw_in.consumed;
                    raw_in.len -= raw_in.consumed;
                    raw_in.consumed = 0;
                }
                else
                {
                    break;
                }

                if (error_count > 40)
                {
                    ESP_LOGE(TAG, "错误次数过多，停止播放");
                    goto cleanup;
                }
                continue;
            }

            if (out_frame.decoded_size > 0)
            {
                int wr = pcm_write(out_frame.buffer, out_frame.decoded_size);
                if (wr < 0)
                    goto cleanup;
                total_pcm += wr;
                frame_count++;
            }

            if (raw_in.consumed > 0)
            {
                raw_in.buffer += raw_in.consumed;
                raw_in.len -= raw_in.consumed;
                raw_in.consumed = 0;
            }
            else
            {
                break;
            }
        }

        if (frame_count % 50 == 0 && frame_count > 0)
        {
            float pct = (float)g_read_ctx.read_offset * 100.0f / g_read_ctx.total_size;
            ESP_LOGI(TAG, "进度: %.1f%%  帧数:%u  PCM:%.1f KB", pct, frame_count, total_pcm / 1024.f);
        }
    }

    uint64_t elapsed_us = esp_timer_get_time() - start_time;
    float elapsed_s = elapsed_us / 1000000.0f;
    float audio_dur = (float)total_pcm / (TARGET_SAMPLE_RATE * TARGET_CHANNELS * (TARGET_BITS_PER_SAMPLE / 8));

    ESP_LOGI(TAG, "========== 播放完成 ==========");
    ESP_LOGI(TAG, "  解码帧数       : %u", frame_count);
    ESP_LOGI(TAG, "  总 PCM 字节    : %llu (~%.1f KB)", total_pcm, total_pcm / 1024.f);
    ESP_LOGI(TAG, "  音频时长       : %.2f 秒", audio_dur);
    ESP_LOGI(TAG, "  处理耗时       : %.2f 秒", elapsed_s);
    ESP_LOGI(TAG, "  实时倍率       : %.2f x", (elapsed_s > 0.001f) ? audio_dur / elapsed_s : 0);
    ESP_LOGI(TAG, "  解码错误次数   : %d", error_count);

cleanup:
    if (decoder)
    {
        esp_audio_dec_close(decoder);
    }
    esp_audio_dec_unregister_default();

    free(in_buf);
    free(out_buf);

inf_loop:
    ESP_LOGI(TAG, "播放结束，进入无限循环...");
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}