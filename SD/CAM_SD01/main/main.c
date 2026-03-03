#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"

#define EXAMPLE_MAX_CHAR_SIZE 64
static const char *TAG = "example";
#define MOUNT_POINT "/sdcard"

// ────────────────────────────────────────────────
//  你之前定义的 sd_write_jpg 函数（序号递增版本）
// ────────────────────────────────────────────────
void sd_write_jpg(const uint8_t *_jpg_buf, size_t _jpg_buf_len)
{
    if (_jpg_buf == NULL || _jpg_buf_len == 0)
    {
        ESP_LOGE(TAG, "Invalid jpeg buffer or length is zero");
        return;
    }

    static uint32_t photo_counter = 1;

    char fname[64];
    snprintf(fname, sizeof(fname), "/sdcard/%04lu.jpg", (unsigned long)photo_counter);

    ESP_LOGI(TAG, "Saving to: %s   size: %u bytes", fname, (unsigned int)_jpg_buf_len);

    FILE *f = fopen(fname, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file: %s", fname);
        return;
    }

    size_t written = fwrite(_jpg_buf, 1, _jpg_buf_len, f);
    if (written != _jpg_buf_len)
    {
        ESP_LOGE(TAG, "Write failed! wanted %u, wrote %u",
                 (unsigned int)_jpg_buf_len, (unsigned int)written);
    }
    else
    {
        ESP_LOGI(TAG, "File written OK → %s", fname);
    }

    fclose(f);

    photo_counter++;
    if (photo_counter > 9999)
        photo_counter = 1;
}

// ────────────────────────────────────────────────
//  app_main 示例：挂载 → 写几张假 jpeg → 卸载
// ────────────────────────────────────────────────
void app_main(void)
{
    esp_err_t ret;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;

    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SDMMC peripheral");

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.clk = GPIO_NUM_39;
    slot_config.cmd = GPIO_NUM_38;
    slot_config.d0 = GPIO_NUM_40;
    slot_config.width = 1;
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                          "Make sure SD card lines have pull-up resistors.",
                     esp_err_to_name(ret));
        }
        return;
    }

    ESP_LOGI(TAG, "Filesystem mounted");
    sdmmc_card_print_info(stdout, card);

    // ──────────────── 测试写入假 jpeg 数据 ────────────────
    // 这里用一小段无效但格式正确的假数据来测试写入功能
    // 真实项目中应传入摄像头拍到的 jpeg 缓冲区
    static const uint8_t fake_jpeg[] = {
        0xFF, 0xD8, // SOI
        0xFF, 0xE0, 0x00, 0x10, 'J', 'F', 'I', 'F', 0x00, 0x01,
        0x01, 0x01, 0x00, 0x48, 0x00, 0x48, 0x00, 0x00,
        0xFF, 0xDB, 0x00, 0x43, 0x00, 0xFF, 0xFF, 0xFF, // 简化量化表
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xC0, 0x00, 0x0B, 0x08, 0x00, 0x08, 0x00, 0x08, 0x01, 0x01, 0x11, 0x00,
        0xFF, 0xDA, 0x00, 0x08, 0x01, 0x01, 0x00, 0x00, 0x3F, 0x00,
        0x47, 0xFF, 0xD9 // EOI
    };

    // 故意写入 3 次，文件名会是 photo_0001.jpg ~ photo_0003.jpg
    ESP_LOGI(TAG, "=== Test writing fake jpeg files ===");
    for (int i = 0; i < 3; i++)
    {
        sd_write_jpg(fake_jpeg, sizeof(fake_jpeg));
        vTaskDelay(pdMS_TO_TICKS(500)); // 稍微间隔一下
    }

    // 做你原来的测试文件操作（可选保留）
    // ... hello.txt / foo.txt / nihao.txt 的代码 ...

    // 最后卸载
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGI(TAG, "Card unmounted");
}