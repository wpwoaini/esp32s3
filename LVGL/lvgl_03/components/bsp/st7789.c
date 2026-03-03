#include <stdio.h>
#include "st7789.h"
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
// #define PIN_NUM_MOSI 36
// #define PIN_NUM_CLK 35
// #define PIN_NUM_CS 45
// #define PIN_NUM_DC 5
// #define PIN_NUM_RST 37
// #define PIN_NUM_BL 6
#define PIN_NUM_BL 1
#define PIN_NUM_CLK 2
#define PIN_NUM_DC 42
#define PIN_NUM_CS 4
#define PIN_NUM_MOSI 5
#define PIN_NUM_RST 6

esp_lcd_panel_handle_t lcd_panel_handle = NULL; // Global panel handle
esp_lcd_panel_io_handle_t io_handle;
void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_draw_bitmap(lcd_panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
    lv_disp_flush_ready(disp);
}

void lvgl_init_display(void)
{
    // static uint8_t lvBuffer1[240 * 10 * 2]; // Buffer for 10 rows
    // static uint8_t lvBuffer2[240 * 10 * 2]; // Buffer for 10 rows
    // Initialize the SPI bus
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_NUM_CLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 240 * 320 * 2 + 8};
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    // Initialize panel IO
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = 40 * 1000 * 1000, // Use 40 MHz for stability
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8};
    esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle);

    // Initialize the display panel
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .color_space = LCD_RGB_ELEMENT_ORDER_RGB, // Use BGR color space
        .bits_per_pixel = 16};

    esp_lcd_new_panel_st7789(io_handle, &panel_config, &lcd_panel_handle);
    // 1. 交换X/Y轴（MV=1）
    esp_lcd_panel_swap_xy(lcd_panel_handle, true);
    // // 2. X轴镜像（MX=1）
    esp_lcd_panel_mirror(lcd_panel_handle, false, true);

    // Reset and initialize the panel
    esp_lcd_panel_reset(lcd_panel_handle);
    esp_lcd_panel_init(lcd_panel_handle);
    esp_lcd_panel_disp_on_off(lcd_panel_handle, true);
    esp_lcd_panel_invert_color(lcd_panel_handle, true); // Disable color inversion

    // // Create LVGL display
    // lv_display_t *lvDisplay = lv_display_create(240, 284);
    // lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565); // Use RGB565

    // lv_display_set_flush_cb(lvDisplay, lvgl_flush_cb);
    // lv_display_set_buffers(lvDisplay, lvBuffer1, lvBuffer2, 240 * 10, LV_DISPLAY_RENDER_MODE_PARTIAL);
}

/** 控制背光
 * @param enable 是否使能背光
 * @return 无
 */
void st7789_lcd_backlight(bool enable)
{
    // Initialize backlight
    gpio_reset_pin(PIN_NUM_BL);
    gpio_set_direction(PIN_NUM_BL, GPIO_MODE_OUTPUT);
    // gpio_set_level(PIN_NUM_BL, 1); // Turn on backlight
    if (enable)
    {
        gpio_set_level(PIN_NUM_BL, 1);
    }
    else
    {
        gpio_set_level(PIN_NUM_BL, 0);
    }
}

void st7789_lcd_set_backlight(uint32_t brightness)
{

    ledc_timer_bit_t fbl = LEDC_TIMER_13_BIT;
    ledc_clk_cfg_t ledc_clk = LEDC_AUTO_CLK;
    ledc_timer_config_t ledc_time_cfg = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5 * 1000,
        .duty_resolution = fbl,
        .clk_cfg = ledc_clk,
    };
    ledc_timer_config(&ledc_time_cfg);

    ledc_channel_t ledc_chan = LEDC_CHANNEL_0;

    ledc_channel_config_t ledc_channel = {
        .gpio_num = PIN_NUM_BL,            // GPIO引脚
        .speed_mode = LEDC_LOW_SPEED_MODE, // 与定时器模式一致
        .channel = ledc_chan,              // 通道编号
        .intr_type = LEDC_INTR_DISABLE,    // 禁用中断
        .timer_sel = LEDC_TIMER_0,         // 选择定时器
        .duty = 50,                        // 初始占空比为0（LED关闭）
        .hpoint = 0,                       // hpoint值
        .sleep_mode = 0,                   // 睡眠模式保持PWM状态
        .flags.output_invert = 0,          // 不反转输出
    };

    ledc_channel_config(&ledc_channel);
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ledc_set_duty(LEDC_LOW_SPEED_MODE, ledc_chan, brightness);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, ledc_chan);
}