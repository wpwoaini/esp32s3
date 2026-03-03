/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_screen_moniter(lv_ui *ui)
{
    //Write codes screen_moniter
    ui->screen_moniter = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_moniter, 320, 240);
    lv_obj_set_scrollbar_mode(ui->screen_moniter, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_moniter, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_moniter, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_moniter, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_moniter, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_cont_mem
    ui->screen_moniter_cont_mem = lv_obj_create(ui->screen_moniter);
    lv_obj_set_pos(ui->screen_moniter_cont_mem, 168, 20);
    lv_obj_set_size(ui->screen_moniter_cont_mem, 140, 110);
    lv_obj_set_scrollbar_mode(ui->screen_moniter_cont_mem, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_moniter_cont_mem, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_cont_mem, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_moniter_cont_mem, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_moniter_cont_mem, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_moniter_cont_mem, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_cont_mem, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_cont_mem, 170, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_moniter_cont_mem, lv_color_hex(0x365996), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_moniter_cont_mem, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_cont_mem, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_cont_mem, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_cont_mem, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_cont_mem, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_cont_mem, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_men2
    ui->screen_moniter_label_men2 = lv_label_create(ui->screen_moniter_cont_mem);
    lv_obj_set_pos(ui->screen_moniter_label_men2, 67, 66);
    lv_obj_set_size(ui->screen_moniter_label_men2, 68, 23);
    lv_label_set_text(ui->screen_moniter_label_men2, "2222MB");
    lv_label_set_long_mode(ui->screen_moniter_label_men2, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_men2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_men2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_men2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_men2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_men2, &lv_font_ZiTiQuanWeiJunHeiW22_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_men2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_men2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_men2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_men2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_men2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_men2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_men2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_men2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_men2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_men2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_mem_title
    ui->screen_moniter_label_mem_title = lv_label_create(ui->screen_moniter_cont_mem);
    lv_obj_set_pos(ui->screen_moniter_label_mem_title, 0, 0);
    lv_obj_set_size(ui->screen_moniter_label_mem_title, 100, 32);
    lv_label_set_text(ui->screen_moniter_label_mem_title, "内存");
    lv_label_set_long_mode(ui->screen_moniter_label_mem_title, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_mem_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_mem_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_mem_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_mem_title, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_mem_title, &lv_font_ZiTiQuanWeiJunHeiW22_36, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_mem_title, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_mem_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_mem_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_mem_title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_mem_title, 117, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_moniter_label_mem_title, lv_color_hex(0x1b5afd), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_moniter_label_mem_title, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_mem_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_mem_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_mem_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_mem_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_mem_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_men_used
    ui->screen_moniter_label_men_used = lv_label_create(ui->screen_moniter_cont_mem);
    lv_obj_set_pos(ui->screen_moniter_label_men_used, 0, 41);
    lv_obj_set_size(ui->screen_moniter_label_men_used, 56, 17);
    lv_label_set_text(ui->screen_moniter_label_men_used, "使用率:");
    lv_label_set_long_mode(ui->screen_moniter_label_men_used, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_men_used, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_men_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_men_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_men_used, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_men_used, &lv_font_ZiTiQuanWeiJunHeiW22_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_men_used, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_men_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_men_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_men_used, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_men_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_men_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_men_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_men_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_men_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_men_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_men_vvalid
    ui->screen_moniter_label_men_vvalid = lv_label_create(ui->screen_moniter_cont_mem);
    lv_obj_set_pos(ui->screen_moniter_label_men_vvalid, 0, 68);
    lv_obj_set_size(ui->screen_moniter_label_men_vvalid, 56, 17);
    lv_label_set_text(ui->screen_moniter_label_men_vvalid, "可用：");
    lv_label_set_long_mode(ui->screen_moniter_label_men_vvalid, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_men_vvalid, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_men_vvalid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_men_vvalid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_men_vvalid, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_men_vvalid, &lv_font_ZiTiQuanWeiJunHeiW22_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_men_vvalid, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_men_vvalid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_men_vvalid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_men_vvalid, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_men_vvalid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_men_vvalid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_men_vvalid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_men_vvalid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_men_vvalid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_men_vvalid, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_men1
    ui->screen_moniter_label_men1 = lv_label_create(ui->screen_moniter_cont_mem);
    lv_obj_set_pos(ui->screen_moniter_label_men1, 93, 37);
    lv_obj_set_size(ui->screen_moniter_label_men1, 42, 23);
    lv_label_set_text(ui->screen_moniter_label_men1, "80%");
    lv_label_set_long_mode(ui->screen_moniter_label_men1, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_men1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_men1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_men1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_men1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_men1, &lv_font_ZiTiQuanWeiJunHeiW22_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_men1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_men1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_men1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_men1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_men1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_men1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_men1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_men1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_men1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_men1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_cont_cpu
    ui->screen_moniter_cont_cpu = lv_obj_create(ui->screen_moniter);
    lv_obj_set_pos(ui->screen_moniter_cont_cpu, 10, 20);
    lv_obj_set_size(ui->screen_moniter_cont_cpu, 140, 110);
    lv_obj_set_scrollbar_mode(ui->screen_moniter_cont_cpu, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_moniter_cont_cpu, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_cont_cpu, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_moniter_cont_cpu, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_moniter_cont_cpu, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_moniter_cont_cpu, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_cont_cpu, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_cont_cpu, 170, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_moniter_cont_cpu, lv_color_hex(0x365996), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_moniter_cont_cpu, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_cont_cpu, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_cont_cpu, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_cont_cpu, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_cont_cpu, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_cont_cpu, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_cpu2
    ui->screen_moniter_label_cpu2 = lv_label_create(ui->screen_moniter_cont_cpu);
    lv_obj_set_pos(ui->screen_moniter_label_cpu2, 95, 67);
    lv_obj_set_size(ui->screen_moniter_label_cpu2, 42, 23);
    lv_label_set_text(ui->screen_moniter_label_cpu2, "20℃");
    lv_label_set_long_mode(ui->screen_moniter_label_cpu2, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_cpu2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_cpu2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_cpu2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_cpu2, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_cpu2, &lv_font_ZiTiQuanWeiJunHeiW22_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_cpu2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_cpu2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_cpu2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_cpu2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_cpu2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_cpu2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_cpu2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_cpu2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_cpu2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_cpu2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_cpu_title
    ui->screen_moniter_label_cpu_title = lv_label_create(ui->screen_moniter_cont_cpu);
    lv_obj_set_pos(ui->screen_moniter_label_cpu_title, 0, 0);
    lv_obj_set_size(ui->screen_moniter_label_cpu_title, 100, 32);
    lv_label_set_text(ui->screen_moniter_label_cpu_title, "CPU");
    lv_label_set_long_mode(ui->screen_moniter_label_cpu_title, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_cpu_title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_cpu_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_cpu_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_cpu_title, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_cpu_title, &lv_font_ZiTiQuanWeiJunHeiW22_36, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_cpu_title, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_cpu_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_cpu_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_cpu_title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_cpu_title, 117, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_moniter_label_cpu_title, lv_color_hex(0x1b5afd), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_moniter_label_cpu_title, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_cpu_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_cpu_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_cpu_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_cpu_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_cpu_title, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_cpu_used
    ui->screen_moniter_label_cpu_used = lv_label_create(ui->screen_moniter_cont_cpu);
    lv_obj_set_pos(ui->screen_moniter_label_cpu_used, 0, 41);
    lv_obj_set_size(ui->screen_moniter_label_cpu_used, 56, 17);
    lv_label_set_text(ui->screen_moniter_label_cpu_used, "使用率:");
    lv_label_set_long_mode(ui->screen_moniter_label_cpu_used, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_cpu_used, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_cpu_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_cpu_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_cpu_used, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_cpu_used, &lv_font_ZiTiQuanWeiJunHeiW22_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_cpu_used, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_cpu_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_cpu_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_cpu_used, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_cpu_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_cpu_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_cpu_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_cpu_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_cpu_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_cpu_used, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_cpu_temp
    ui->screen_moniter_label_cpu_temp = lv_label_create(ui->screen_moniter_cont_cpu);
    lv_obj_set_pos(ui->screen_moniter_label_cpu_temp, 0, 68);
    lv_obj_set_size(ui->screen_moniter_label_cpu_temp, 56, 17);
    lv_label_set_text(ui->screen_moniter_label_cpu_temp, "温度:");
    lv_label_set_long_mode(ui->screen_moniter_label_cpu_temp, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_cpu_temp, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_cpu_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_cpu_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_cpu_temp, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_cpu_temp, &lv_font_ZiTiQuanWeiJunHeiW22_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_cpu_temp, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_cpu_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_cpu_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_cpu_temp, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_cpu_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_cpu_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_cpu_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_cpu_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_cpu_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_cpu_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_cpu1
    ui->screen_moniter_label_cpu1 = lv_label_create(ui->screen_moniter_cont_cpu);
    lv_obj_set_pos(ui->screen_moniter_label_cpu1, 93, 37);
    lv_obj_set_size(ui->screen_moniter_label_cpu1, 42, 23);
    lv_label_set_text(ui->screen_moniter_label_cpu1, "80%");
    lv_label_set_long_mode(ui->screen_moniter_label_cpu1, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_cpu1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_cpu1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_cpu1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_cpu1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_cpu1, &lv_font_ZiTiQuanWeiJunHeiW22_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_cpu1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_cpu1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_cpu1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_cpu1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_cpu1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_cpu1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_cpu1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_cpu1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_cpu1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_cpu1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_ta_ip
    ui->screen_moniter_ta_ip = lv_textarea_create(ui->screen_moniter);
    lv_obj_set_pos(ui->screen_moniter_ta_ip, 84, 148);
    lv_obj_set_size(ui->screen_moniter_ta_ip, 150, 28);
    lv_textarea_set_text(ui->screen_moniter_ta_ip, "192.168.0.1");
    lv_textarea_set_placeholder_text(ui->screen_moniter_ta_ip, "");
    lv_textarea_set_password_bullet(ui->screen_moniter_ta_ip, "*");
    lv_textarea_set_password_mode(ui->screen_moniter_ta_ip, false);
    lv_textarea_set_one_line(ui->screen_moniter_ta_ip, false);
    lv_textarea_set_accepted_chars(ui->screen_moniter_ta_ip, "");
    lv_textarea_set_max_length(ui->screen_moniter_ta_ip, 32);
#if LV_USE_KEYBOARD
    lv_obj_add_event_cb(ui->screen_moniter_ta_ip, ta_event_cb, LV_EVENT_ALL, ui->g_kb_top_layer);
#endif

    //Write style for screen_moniter_ta_ip, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->screen_moniter_ta_ip, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_ta_ip, &lv_font_ZiTiQuanWeiJunHeiW22_18, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_ta_ip, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_ta_ip, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_ta_ip, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_ta_ip, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_moniter_ta_ip, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_moniter_ta_ip, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_moniter_ta_ip, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_moniter_ta_ip, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_moniter_ta_ip, lv_color_hex(0xe6e6e6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_moniter_ta_ip, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_ta_ip, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_ta_ip, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_ta_ip, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_ta_ip, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_ta_ip, 4, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for screen_moniter_ta_ip, Part: LV_PART_SCROLLBAR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_moniter_ta_ip, 255, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_moniter_ta_ip, lv_color_hex(0x2195f6), LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_moniter_ta_ip, LV_GRAD_DIR_NONE, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_ta_ip, 0, LV_PART_SCROLLBAR|LV_STATE_DEFAULT);

    //Write codes screen_moniter_btn_connect
    ui->screen_moniter_btn_connect = lv_button_create(ui->screen_moniter);
    lv_obj_set_pos(ui->screen_moniter_btn_connect, 248, 151);
    lv_obj_set_size(ui->screen_moniter_btn_connect, 60, 25);
    ui->screen_moniter_btn_connect_label = lv_label_create(ui->screen_moniter_btn_connect);
    lv_label_set_text(ui->screen_moniter_btn_connect_label, "连接");
    lv_label_set_long_mode(ui->screen_moniter_btn_connect_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_moniter_btn_connect_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_moniter_btn_connect, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_moniter_btn_connect_label, LV_PCT(100));

    //Write style for screen_moniter_btn_connect, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_moniter_btn_connect, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_moniter_btn_connect, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_moniter_btn_connect, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_moniter_btn_connect, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_btn_connect, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_btn_connect, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_btn_connect, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_btn_connect, &lv_font_ZiTiQuanWeiJunHeiW22_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_btn_connect, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_btn_connect, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_host
    ui->screen_moniter_label_host = lv_label_create(ui->screen_moniter);
    lv_obj_set_pos(ui->screen_moniter_label_host, 27, 151);
    lv_obj_set_size(ui->screen_moniter_label_host, 61, 25);
    lv_label_set_text(ui->screen_moniter_label_host, "主机:");
    lv_label_set_long_mode(ui->screen_moniter_label_host, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_host, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_host, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_host, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_host, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_host, &lv_font_ZiTiQuanWeiJunHeiW22_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_host, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_host, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_host, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_host, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_host, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_host, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_host, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_host, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_host, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_host, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_moniter_label_state
    ui->screen_moniter_label_state = lv_label_create(ui->screen_moniter);
    lv_obj_set_pos(ui->screen_moniter_label_state, 134, 193);
    lv_obj_set_size(ui->screen_moniter_label_state, 63, 15);
    lv_label_set_text(ui->screen_moniter_label_state, "未连接");
    lv_label_set_long_mode(ui->screen_moniter_label_state, LV_LABEL_LONG_WRAP);

    //Write style for screen_moniter_label_state, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_moniter_label_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_moniter_label_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_moniter_label_state, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_moniter_label_state, &lv_font_ZiTiQuanWeiJunHeiW22_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_moniter_label_state, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_moniter_label_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_moniter_label_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_moniter_label_state, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_moniter_label_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_moniter_label_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_moniter_label_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_moniter_label_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_moniter_label_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_moniter_label_state, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen_moniter.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_moniter);

    //Init events for screen.
    events_init_screen_moniter(ui);
}
