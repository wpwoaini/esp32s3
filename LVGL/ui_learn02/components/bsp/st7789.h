#ifndef ST7789_H
#define ST7789_H
#include "lvgl.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
extern esp_lcd_panel_handle_t lcd_panel_handle;

// void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void lvgl_init_display(void);
void st7789_lcd_backlight(bool enable);
void st7789_lcd_set_backlight(uint32_t brightness);
#endif