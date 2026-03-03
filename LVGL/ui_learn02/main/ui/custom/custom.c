/*
 * Copyright 2024 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl.h"
#include "custom.h"
#include "esp_lvgl_port.h"
#include "time.h"
#include "string.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**
 * Create a demo application
 */
extern int screen_home_digital_clock_now_hour_value;
extern int screen_home_digital_clock_now_min_value;
extern int screen_home_digital_clock_now_sec_value;
int screen_home_digital_clock_now_year_value;
int screen_home_digital_clock_now_month_value;
int screen_home_digital_clock_now_day_value;
int screen_home_digital_clock_now_w_day_value;
static const char *week_days[] = {"日", "一", "二", "三", "四", "五", "六"};
void data_count(void)
{
    time_t now = time(NULL);
    struct tm t;
    localtime_r(&now, &t);
    screen_home_digital_clock_now_year_value = t.tm_year + 1900;
    screen_home_digital_clock_now_month_value = t.tm_mon + 1;
    screen_home_digital_clock_now_day_value = t.tm_mday;
    screen_home_digital_clock_now_w_day_value = t.tm_wday;

    lv_label_set_text_fmt(guider_ui.screen_home_label_data, "%04d年%02d月%02d日", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    lv_label_set_text_fmt(guider_ui.screen_home_label_week, "星期%s", week_days[t.tm_wday]);
}

void set_home_time(lv_ui *ui, int year, int month, int day, int w_day, int hour, int min, int sec)
{
    screen_home_digital_clock_now_year_value = year;
    screen_home_digital_clock_now_month_value = month;
    screen_home_digital_clock_now_day_value = day;
    screen_home_digital_clock_now_w_day_value = w_day;
    screen_home_digital_clock_now_hour_value = hour;
    screen_home_digital_clock_now_min_value = min;
    screen_home_digital_clock_now_sec_value = sec;
    lv_label_set_text_fmt(ui->screen_home_label_data, "%04d年%02d月%02d日", year, month, day);
    lv_label_set_text_fmt(guider_ui.screen_home_label_week, "星期%s", week_days[w_day]);
}

/**
 * @brief 设置首页天气信息
 * @param ui 用户界面结构体指针
 * @param weather_data 天气数据结构体
 */
void set_home_weather(lv_ui *ui, weather_data_t weather_data)
{
    // 获取天气数据
    weather_get_data(&weather_data);
    // 设置城市名称
    lv_label_set_text_fmt(ui->screen_home_label_city, "%s", weather_data.district);

    lv_label_set_text_fmt(ui->screen_home_label_today, "今日:%s", weather_data.forecast[0].weather_day);
    lv_label_set_long_mode(ui->screen_home_label_today, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text_fmt(ui->screen_home_label_temp1, "%d-%d℃", weather_data.forecast[0].temp_min, weather_data.forecast[0].temp_max);

    lv_label_set_text_fmt(ui->screen_home_label_tomo, "明日:%s", weather_data.forecast[1].weather_day);
    lv_label_set_long_mode(ui->screen_home_label_tomo, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text_fmt(ui->screen_home_label_temp2, "%d-%d℃", weather_data.forecast[1].temp_min, weather_data.forecast[1].temp_max);

    lv_label_set_text_fmt(ui->screen_home_label_after, "后日:%s", weather_data.forecast[2].weather_day);
    lv_label_set_long_mode(ui->screen_home_label_after, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_label_set_text_fmt(ui->screen_home_label_temp3, "%d-%d℃", weather_data.forecast[2].temp_min, weather_data.forecast[2].temp_max);
    lv_obj_t *weather_img_objs[] = {
        ui->screen_home_img_today, // 今日图标控件
        ui->screen_home_img_tomo,  // 明天图标控件
        ui->screen_home_img_after  // 后天图标控件
    };
    for (size_t i = 0; i < 3; i++)
    {
        // 1. 安全校验：确保控件对象和天气数据有效
        if (weather_img_objs[i] == NULL || weather_data.forecast[i].weather_day[0] == '\0')
        {
            continue; // 跳过空控件/空数据
        }

        // 2. 获取当天的天气文字
        char *weather_text = weather_data.forecast[i].weather_day;

        // 3. 匹配关键词并设置对应图标（优先级：雨 > 晴 > 云）
        if (strstr(weather_text, "雨") != NULL)
        {
            lv_image_set_src(weather_img_objs[i], &_yu_RGB565A8_48x48); // 雨图标
        }
        else if (strstr(weather_text, "晴") != NULL)
        {
            lv_image_set_src(weather_img_objs[i], &_qing_RGB565A8_48x48); // 晴图标
        }
        else if (strstr(weather_text, "云") != NULL)
        {
            lv_image_set_src(weather_img_objs[i], &_yun_RGB565A8_48x48); // 云/多云图标
        }
        else
        {
            lv_image_set_src(weather_img_objs[i], &_yun_RGB565A8_48x48); // 默认图标（云）
        }
    }
}

void custom_init(lv_ui *ui)
{
    /* Add your codes here */
}
