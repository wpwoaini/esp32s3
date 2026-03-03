#ifndef SD_H
#define SD_H
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "esp_camera.h"
void sd_init(void);
void sd_write_jpg(const uint8_t *_jpg_buf, size_t _jpg_buf_len, uint32_t photo_counter);
esp_err_t sd_write_file(const char *path, char *data);
esp_err_t sd_read_file(const char *path);
esp_err_t sd_read_jpg_file(const char *path, uint8_t **out_data, size_t *out_size);
esp_err_t sd_save_jpg(bool read);
esp_err_t sd_read_jpg(uint32_t photo_counter);
#endif