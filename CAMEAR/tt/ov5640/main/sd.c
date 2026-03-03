#include "sd.h"

static const char *TAG = "SD";
#define MOUNT_POINT "/sdcard"
#define EXAMPLE_MAX_CHAR_SIZE 64

#define EXAMPLE_MAX_JPG_SIZE (1024 * 1024) // 最大支持1MB的JPG图片
// static uint32_t photo_counter = 1;
static uint32_t jpg_i = 1;
esp_err_t sd_save_jpg(bool read)
{
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();
    sd_write_jpg(fb->buf, fb->len, jpg_i);
    esp_camera_fb_return(fb);
    if (read)
    {
        sd_read_jpg(jpg_i);
    }

    jpg_i++;
    // sd_read_file("/sdcard/photo_counter.jpg");
    // const char *jpg_path = "/sdcard/photo_counter.jpg"; // SD卡上的JPG文件路径
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
    //     ESP_LOGE(TAG, "Failed to read JPG file, err: %d", reta);
    // }
    return ESP_OK;
}

esp_err_t sd_read_jpg(uint32_t photo_counter)
{
    // 1. 定义文件名缓冲区，拼接全局变量photo_counter
    char jpg_path[64]; // 足够存储路径+数字的缓冲区
    snprintf(jpg_path, sizeof(jpg_path), "/sdcard/%04lu.jpg", (unsigned long)photo_counter);
    // snprintf(fname, sizeof(fname), "/sdcard/%04lu.jpg", (unsigned long)photo_counter);
    uint8_t *jpg_data = NULL;
    size_t jpg_size = 0;

    // 2. 调用读取函数
    esp_err_t reta = sd_read_jpg_file(jpg_path, &jpg_data, &jpg_size);
    if (reta == ESP_OK)
    {
        // 处理图片数据（打印日志+16进制数据）
        ESP_LOGI(TAG, "JPG data ready, size: %zu bytes", jpg_size);
        // 注意：ESP_LOG_BUFFER_HEX不宜打印大文件，建议限制打印长度（比如前64字节）
        // ESP_LOG_BUFFER_HEX(TAG, jpg_data, jpg_size > 64 ? 64 : jpg_size);
        ESP_LOG_BUFFER_HEX(TAG, jpg_data, jpg_size);

        // 3. 用完后释放内存（关键！避免内存泄漏）
        free(jpg_data);
        jpg_data = NULL;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to read JPG file (photo_counter: %d), err: %d", photo_counter, reta);
    }

    return reta;
}

/**
 * @brief 从SD卡读取JPG图片文件的二进制数据
 * @param path 图片文件路径
 * @param out_data 输出参数，指向存储图片数据的缓冲区（需要外部释放）
 * @param out_size 输出参数，返回读取到的图片数据大小
 * @return esp_err_t ESP_OK成功，其他失败
 */
esp_err_t sd_read_jpg_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    ESP_LOGI(TAG, "Reading JPG file %s", path);

    // 校验输入参数
    if (path == NULL || out_data == NULL || out_size == NULL)
    {
        ESP_LOGE(TAG, "Invalid input parameters");
        return ESP_ERR_INVALID_ARG;
    }

    // 1. 以二进制模式打开文件
    FILE *f = fopen(path, "rb"); // "rb" 表示二进制读取模式
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open JPG file for reading");
        return ESP_FAIL;
    }

    // 2. 获取文件大小
    struct stat file_stat;
    if (fstat(fileno(f), &file_stat) != 0)
    {
        ESP_LOGE(TAG, "Failed to get file stat");
        fclose(f);
        return ESP_FAIL;
    }
    size_t file_size = file_stat.st_size;

    // 检查文件大小是否超出限制
    if (file_size == 0 || file_size > EXAMPLE_MAX_JPG_SIZE)
    {
        ESP_LOGE(TAG, "File size is invalid (size: %zu)", file_size);
        fclose(f);
        return ESP_ERR_INVALID_SIZE;
    }

    // 3. 分配内存存储图片数据
    *out_data = (uint8_t *)malloc(file_size);
    if (*out_data == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for JPG data");
        fclose(f);
        return ESP_ERR_NO_MEM;
    }

    // 4. 读取完整的二进制数据
    size_t read_size = fread(*out_data, 1, file_size, f);
    if (read_size != file_size)
    {
        ESP_LOGE(TAG, "Failed to read full JPG data (read: %zu, expected: %zu)", read_size, file_size);
        free(*out_data);
        *out_data = NULL;
        fclose(f);
        return ESP_FAIL;
    }

    // 5. 关闭文件，设置输出参数
    fclose(f);
    *out_size = file_size;

    ESP_LOGI(TAG, "Successfully read JPG file: %s, size: %zu bytes", path, file_size);
    return ESP_OK;
}

esp_err_t sd_write_file(const char *path, char *data)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    return ESP_OK;
}

esp_err_t sd_read_file(const char *path)
{
    ESP_LOGI(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // strip newline
    char *pos = strchr(line, '\n');
    if (pos)
    {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    return ESP_OK;
}

void sd_write_jpg(const uint8_t *_jpg_buf, size_t _jpg_buf_len, uint32_t photo_counter)
{
    if (_jpg_buf == NULL || _jpg_buf_len == 0)
    {
        ESP_LOGE(TAG, "Invalid jpeg buffer or length is zero");
        return;
    }

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
void sd_init(void)
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
}