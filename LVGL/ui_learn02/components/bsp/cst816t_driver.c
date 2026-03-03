#include "cst816t_driver.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>

#define TOUCH_I2C_PORT I2C_NUM_0 // I2C端口号
#define CST816T_ADDR 0x15        // 设备I2C地址
#define TOUCH_TIMEOUT_MS 100     // I2C操作超时时间（ms）
#define RETRY_MAX 3              // 通信重试次数
#define RECONNECT_INTERVAL 5000  // 断线重连检测间隔（ms）

static const char *TAG = "cst816t";

// 静态变量（仅内部使用）
static uint16_t s_usLimitX = 0;                       // X轴边界值
static uint16_t s_usLimitY = 0;                       // Y轴边界值
static i2c_master_bus_handle_t i2c_mas_handle = NULL; // I2C总线句柄
static i2c_master_dev_handle_t i2c_dev_handle = NULL; // 设备句柄
static bool s_device_connected = false;               // 设备在线标记
static TickType_t s_last_reconnect_try = 0;           // 上次重连时间

/**
 * @brief 检测设备是否在线（通过I2C地址探测）
 * @return true: 在线，false: 离线
 */
static bool cst816t_check_connected(void)
{
    if (i2c_mas_handle == NULL)
    {
        return false;
    }

    // 探测设备地址（带超时，不阻塞）
    esp_err_t err = i2c_master_probe(
        i2c_mas_handle,
        CST816T_ADDR,
        pdMS_TO_TICKS(TOUCH_TIMEOUT_MS));
    return (err == ESP_OK);
}

/**
 * @brief 带重试的I2C读寄存器操作
 * @param reg 寄存器地址
 * @param rx_buf 接收缓冲区
 * @param rx_len 接收长度
 * @return ESP_OK: 成功，其他: 失败
 */
static esp_err_t cst816t_i2c_read_reg(uint8_t reg, uint8_t *rx_buf, size_t rx_len)
{
    if (!s_device_connected || i2c_dev_handle == NULL)
    {
        return ESP_ERR_NOT_FOUND;
    }

    // 多次重试，应对临时通信错误
    for (int retry = 0; retry < RETRY_MAX; retry++)
    {
        esp_err_t err = i2c_master_transmit_receive(
            i2c_dev_handle,
            &reg, 1,                        // 寄存器地址（长度1）
            rx_buf, rx_len,                 // 接收缓冲区和长度
            pdMS_TO_TICKS(TOUCH_TIMEOUT_MS) // 超时控制
        );
        if (err == ESP_OK)
        {
            return ESP_OK;
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // 重试间隔10ms
    }

    // 多次失败，标记设备离线
    s_device_connected = false;
    ESP_LOGE(TAG, "I2C通信失败，设备标记为离线");
    return ESP_ERR_TIMEOUT; // 使用标准错误码
}

/**
 * @brief 尝试重新连接设备（间隔控制，降低CPU占用）
 */
static void cst816t_try_reconnect(void)
{
    // 检查重连间隔，避免频繁探测
    if (xTaskGetTickCount() - s_last_reconnect_try < pdMS_TO_TICKS(RECONNECT_INTERVAL))
    {
        return;
    }
    s_last_reconnect_try = xTaskGetTickCount();

    // ESP_LOGI(TAG, "尝试重新连接CST816T...");
    if (cst816t_check_connected())
    {
        // 设备已连接，重建设备句柄
        i2c_device_config_t i2c_dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = CST816T_ADDR,
            .scl_speed_hz = 400000, // 与初始化频率一致
            .scl_wait_us = 0,
        };
        esp_err_t err = i2c_master_bus_add_device(
            i2c_mas_handle,
            &i2c_dev_cfg,
            &i2c_dev_handle);
        if (err == ESP_OK)
        {
            s_device_connected = true;
            ESP_LOGI(TAG, "CST816T重新连接成功");
        }
        else
        {
            ESP_LOGE(TAG, "重建设备句柄失败: 0x%x", err);
            i2c_dev_handle = NULL;
        }
    }
}

/**
 * @brief 初始化CST816T触摸芯片
 * @param cfg 配置参数
 * @return ESP_OK: 成功，其他: 失败
 */
esp_err_t cst816t_init(cst816t_cfg_t *cfg)
{
    // 避免重复初始化
    if (i2c_mas_handle != NULL && s_device_connected)
    {
        return ESP_OK;
    }

    // 释放旧资源
    if (i2c_dev_handle != NULL)
    {
        i2c_master_bus_rm_device(i2c_dev_handle);
        i2c_dev_handle = NULL;
    }
    if (i2c_mas_handle != NULL)
    {
        i2c_master_bus_reset(i2c_mas_handle);
        i2c_mas_handle = NULL;
    }
    s_device_connected = false;

    // 初始化I2C总线（带重试）
    i2c_master_bus_config_t i2c_mas_cfg = {
        .i2c_port = TOUCH_I2C_PORT,
        .sda_io_num = cfg->sda,
        .scl_io_num = cfg->scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 1,
        .flags.enable_internal_pullup = 1, // 启用内部上拉
    };

    esp_err_t err;
    for (int retry = 0; retry < RETRY_MAX; retry++)
    {
        err = i2c_new_master_bus(&i2c_mas_cfg, &i2c_mas_handle);
        if (err == ESP_OK)
        {
            break;
        }
        ESP_LOGE(TAG, "I2C总线初始化失败（重试 %d/%d）: 0x%x", retry + 1, RETRY_MAX, err);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C总线初始化最终失败");
        return err;
    }

    // 保存屏幕边界值
    s_usLimitX = cfg->x_limit;
    s_usLimitY = cfg->y_limit;

    // 检测设备是否在线（首次初始化）
    if (cst816t_check_connected())
    {
        // 创建设备句柄
        i2c_device_config_t i2c_dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = CST816T_ADDR,
            .scl_speed_hz = cfg->fre,
            .scl_wait_us = 0,
        };
        err = i2c_master_bus_add_device(
            i2c_mas_handle,
            &i2c_dev_cfg,
            &i2c_dev_handle);
        if (err == ESP_OK)
        {
            s_device_connected = true;
            ESP_LOGI(TAG, "CST816T初始化成功");
            return ESP_OK;
        }
        else
        {
            ESP_LOGE(TAG, "创建设备句柄失败: 0x%x", err);
            i2c_dev_handle = NULL;
        }
    }

    // 设备未连接，初始化仍视为成功（系统继续运行）
    ESP_LOGW(TAG, "CST816T未连接，等待设备接入");
    return ESP_OK;
}

/**
 * @brief 读取触摸坐标和状态（低CPU占用）
 * @param x 输出X坐标
 * @param y 输出Y坐标
 * @param state 输出触摸状态（0: 松手，1: 按下）
 */
void cst816t_read(int16_t *x, int16_t *y, int *state)
{
    // 静态变量保存上次有效坐标（避免突变）
    static int16_t last_x = 0;
    static int16_t last_y = 0;

    // 初始化返回值（默认松手状态）
    *x = last_x;
    *y = last_y;
    *state = 0;

    // 检查输入指针有效性
    if (x == NULL || y == NULL || state == NULL)
    {
        ESP_LOGE(TAG, "输入指针为NULL");
        return;
    }

    // I2C总线未初始化，直接返回
    if (i2c_mas_handle == NULL)
    {
        return;
    }

    // 设备离线时，尝试重连（间隔控制）
    if (!s_device_connected)
    {
        cst816t_try_reconnect();
        return;
    }

    // 读取触摸点数量（1个触摸点才有效）
    uint8_t touch_pnt_cnt = 0;
    esp_err_t err = cst816t_i2c_read_reg(0x02, &touch_pnt_cnt, 1);
    if (err != ESP_OK || touch_pnt_cnt != 1)
    {
        return; // 无触摸或多点触摸，返回松手状态
    }

    // 读取X坐标（2字节）
    uint8_t data_x[2] = {0};
    if (cst816t_i2c_read_reg(0x03, data_x, 2) != ESP_OK)
    {
        return;
    }

    // 读取Y坐标（2字节）
    uint8_t data_y[2] = {0};
    if (cst816t_i2c_read_reg(0x05, data_y, 2) != ESP_OK)
    {
        return;
    }

    // 解析坐标（12位有效数据）
    int16_t current_x = ((data_x[0] & 0x0F) << 8) | data_x[1];
    int16_t current_y = ((data_y[0] & 0x0F) << 8) | data_y[1];

    // 坐标转换（根据屏幕方向调整）
    current_x = (s_usLimitX - 1) - current_x; // X轴反转
    const int16_t Y_OFFSET = 25;              // Y轴偏移（根据硬件调整）
    current_y -= Y_OFFSET;

    // 坐标限位（确保在屏幕范围内）
    current_x = (current_x < 0) ? 0 : (current_x >= s_usLimitX) ? (s_usLimitX - 1)
                                                                : current_x;
    current_y = (current_y < 0) ? 0 : (current_y >= s_usLimitY) ? (s_usLimitY - 1)
                                                                : current_y;

    // 更新上次坐标（仅在变化时）
    if (current_x != last_x || current_y != last_y)
    {
        last_x = current_x;
        last_y = current_y;
        ESP_LOGD(TAG, "触摸坐标: x=%d, y=%d", last_x, last_y); // 调试日志
    }

    // 返回当前状态
    *x = last_x;
    *y = last_y;
    *state = 1; // 按下状态
}

// /** 读取坐标值  1
//  * @param  x x坐标
//  * @param  y y坐标
//  * @param state 松手状态 0,松手 1按下
//  * @return 无
// */
// void cst816t_read(int16_t *x,int16_t *y,int *state)
// {
//     uint8_t data_x[2];          // 2 bytesX
//     uint8_t data_y[2];          // 2 bytesY
//     uint8_t touch_pnt_cnt = 0;          // Number of detected touch points
//     static int16_t last_x = 0;  // 12bit pixel value
//     static int16_t last_y = 0;  // 12bit pixel value
//     ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_dev_handle, (const uint8_t[]){0x02}, 1, &touch_pnt_cnt, 1, pdMS_TO_TICKS(1000)));

//     if (touch_pnt_cnt != 1) {    // ignore no touch & multi touch
//         *x = last_x;
//         *y = last_y;
//         *state = 0;
//         return;
//     }

//     //读取X坐标
//     ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_dev_handle, (const uint8_t[]){0x03}, 1, data_x, 2, pdMS_TO_TICKS(1000)));

//     //读取Y坐标
//     ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_dev_handle, (const uint8_t[]){0x05}, 1, data_y, 2, pdMS_TO_TICKS(1000)));

//     int16_t current_x = ((data_x[0] & 0x0F) << 8) | (data_x[1] & 0xFF);
//     int16_t current_y = ((data_y[0] & 0x0F) << 8) | (data_y[1] & 0xFF);

//     // **新增代码：反转 Y 坐标**
//     // 假设 s_usLimitY 是显示屏的垂直分辨率（高度，例如 320 或 240）
//     current_y = (s_usLimitY - 1) - current_y;

//     if(last_x != current_x || current_y != last_y)
//     {
//         last_x = current_x;
//         last_y = current_y;
//         //ESP_LOGI(TAG,"touch x:%d,y:%d",last_x,last_y);
//     }

//     if(last_x >= s_usLimitX)
//         last_x = s_usLimitX - 1;
//     if(last_y >= s_usLimitY)
//         last_y = s_usLimitY - 1;

//     *x = last_x;
//     *y = last_y;
//     *state = 1;
// }

// /** 读取坐标值
//  * @param  x x坐标
//  * @param  y y坐标
//  * @param state 松手状态 0,松手 1按下
//  * @return 无
// */
// void cst816t_read(int16_t *x,int16_t *y,int *state)
// {
//     uint8_t data_x[2];        // 2 bytesX
//     uint8_t data_y[2];        // 2 bytesY
//     uint8_t touch_pnt_cnt = 0;        // Number of detected touch points
//     static int16_t last_x = 0;  // 12bit pixel value
//     static int16_t last_y = 0;  // 12bit pixel value
//     ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_dev_handle, (const uint8_t[]){0x02}, 1, &touch_pnt_cnt, 1, pdMS_TO_TICKS(1000)));

//     if (touch_pnt_cnt != 1) {    // ignore no touch & multi touch
//         *x = last_x;
//         *y = last_y;
//         *state = 0;
//         return;
//     }

//     //读取X坐标
//     ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_dev_handle, (const uint8_t[]){0x03}, 1, data_x, 2, pdMS_TO_TICKS(1000)));

//     //读取Y坐标
//     ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_dev_handle, (const uint8_t[]){0x05}, 1, data_y, 2, pdMS_TO_TICKS(1000)));

//     int16_t current_x = ((data_x[0] & 0x0F) << 8) | (data_x[1] & 0xFF);
//     int16_t current_y = ((data_y[0] & 0x0F) << 8) | (data_y[1] & 0xFF);

//     if(last_x != current_x || current_y != last_y)
//     {
//         last_x = current_x;
//         last_y = current_y;
//         //ESP_LOGI(TAG,"touch x:%d,y:%d",last_x,last_y);
//     }

//     if(last_x >= s_usLimitX)
//         last_x = s_usLimitX - 1;
//     if(last_y >= s_usLimitY)
//         last_y = s_usLimitY - 1;

//     *x = last_x;
//     *y = last_y;
//     *state = 1;
// }
