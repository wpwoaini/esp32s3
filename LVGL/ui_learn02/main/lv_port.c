#include "lv_port.h"
#include "esp_timer.h"
#include "esp_log.h"

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t *disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    // /* Example 1
    //  * One buffer for partial rendering*/
    // LV_ATTRIBUTE_MEM_ALIGN
    // static uint8_t buf_1_1[MY_DISP_HOR_RES * 10 * BYTE_PER_PIXEL];            /*A buffer for 10 rows*/
    // lv_display_set_buffers(disp, buf_1_1, NULL, sizeof(buf_1_1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // /* Example 2
    //  * Two buffers for partial rendering
    //  * In flush_cb DMA or similar hardware should be used to update the display in the background.*/
    // LV_ATTRIBUTE_MEM_ALIGN
    // static uint8_t buf_2_1[MY_DISP_HOR_RES * 10 * BYTE_PER_PIXEL];

    // LV_ATTRIBUTE_MEM_ALIGN
    // static uint8_t buf_2_2[MY_DISP_HOR_RES * 10 * BYTE_PER_PIXEL];
    // lv_display_set_buffers(disp, buf_2_1, buf_2_2, sizeof(buf_2_1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // /* Example 3
    //  * Two buffers screen sized buffer for double buffering.
    //  * Both LV_DISPLAY_RENDER_MODE_DIRECT and LV_DISPLAY_RENDER_MODE_FULL works, see their comments*/
    // LV_ATTRIBUTE_MEM_ALIGN
    // static uint8_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL];

    // LV_ATTRIBUTE_MEM_ALIGN
    // static uint8_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL];
    // lv_display_set_buffers(disp, buf_3_1, buf_3_2, sizeof(buf_3_1), LV_DISPLAY_RENDER_MODE_DIRECT);

    // 声明为指针类型

    LV_ATTRIBUTE_MEM_ALIGN
    uint8_t *buf_2_1 = heap_caps_malloc(MY_DISP_HOR_RES * 24 * BYTE_PER_PIXEL, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);

    LV_ATTRIBUTE_MEM_ALIGN
    uint8_t *buf_2_2 = heap_caps_malloc(MY_DISP_HOR_RES * 24 * BYTE_PER_PIXEL, MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);

    // 计算正确的缓冲区大小（移除多余的 sizeof(uint8_t)）
    size_t buf_size = MY_DISP_HOR_RES * 24 * BYTE_PER_PIXEL;

    // 检查内存分配是否成功
    if (buf_2_1 == NULL || buf_2_2 == NULL)
    {
        ESP_LOGE("存储", "LVGL buffer allocation failed!");
        return; // 或进行错误处理
    }

    // 设置显示缓冲区
    lv_display_set_buffers(disp, buf_2_1, buf_2_2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
    lvgl_init_display();
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the display.
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_display_flush_ready()' has to be called when it's finished.*/
/**
 * 交换RGB565缓冲区的高低字节
 * 适用于ST7789V在SPI/8位并行等接口下的字节序不匹配问题
 * 将RRRRRGGG|GGGBBBBB转换为GGGBBBBB|RRRRRGGG
 *
 * @param buf      RGB565像素数据缓冲区
 * @param px_count 像素数量（非字节数）
 */
// static void rgb565_swap_bytes(uint8_t *buf, uint32_t px_count) {
//     if(buf == NULL || px_count == 0) return;

//     uint16_t *px_data = (uint16_t *)buf;
//     for(uint32_t i = 0; i < px_count; i++) {
//         // 交换16位像素的高低字节
//         px_data[i] = (px_data[i] << 8) | (px_data[i] >> 8);
//     }
// }

// 在刷新回调中调用
static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map)
{
    // 计算刷新区域的像素数量
    uint32_t width = area->x2 - area->x1 + 1;
    uint32_t height = area->y2 - area->y1 + 1;
    uint32_t px_count = width * height;

    if (disp_flush_enabled)
    {
        // 计算刷新区域的像素数量

        // 交换字节序（针对ST7789V的接口字节序要求）
        // rgb565_swap_bytes(px_map, px_count);
        // 交换字节序（针对ST7789V的接口字节序要求）
        lv_draw_sw_rgb565_swap(px_map, px_count);
        // 发送处理后的像素数据到屏幕
        esp_lcd_panel_draw_bitmap(lcd_panel_handle,
                                  area->x1 + 36, area->y1,
                                  area->x2 + 1 + 36, area->y2 + 1,
                                  px_map);
    }

    lv_display_flush_ready(disp_drv);
}

static void lv_tick_inc_cb(void *data)
{
    uint32_t tick_inc_period_ms = *((uint32_t *)data);
    lv_tick_inc(tick_inc_period_ms);
}

/**
 * @brief 创建LVGL定时器
 *
 * @return esp_err_t
 */
static esp_err_t lv_port_tick_init(void)
{
    static uint32_t tick_inc_period_ms = 5;
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = lv_tick_inc_cb,
        .name = "",
        .arg = &tick_inc_period_ms,
        .dispatch_method = ESP_TIMER_TASK,
        .skip_unhandled_events = true,
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, tick_inc_period_ms * 1000));

    return ESP_OK;
}

static void touchpad_init(void);
static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data);
static bool touchpad_is_pressed(void);
static void touchpad_get_xy(int32_t *x, int32_t *y);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t *indev_touchpad;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void)
{
    /*------------------
     * Touchpad
     * -----------------*/

    /*Initialize your touchpad if you have*/

    /*Register a touchpad input device*/
    indev_touchpad = lv_indev_create();
    lv_indev_set_type(indev_touchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_touchpad, touchpad_read);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*------------------
 * Touchpad
 * -----------------*/

/*Initialize your touchpad*/
static void touchpad_init(void)
{
    /*Your code comes here*/
    cst816t_cfg_t cst816t_config = {
        .scl = GPIO_NUM_16,
        .sda = GPIO_NUM_15,
        .x_limit = MY_DISP_VER_RES,
        .y_limit = MY_DISP_HOR_RES,
        .fre = 200 * 1000,
    };
    cst816t_init(&cst816t_config);
}

#define PIN_NUM_CLK 2
#define PIN_NUM_DC 42
#define PIN_NUM_CS 4
#define PIN_NUM_MOSI 5
#define PIN_NUM_RST 6
scl = GPIO_NUM_16,
    sda = GPIO_NUM_15,

    /*Will be called by the library to read the touchpad*/
    static void touchpad_read(lv_indev_t *indev_drv, lv_indev_data_t *data)
{
    // static int32_t last_x = 0;
    // static int32_t last_y = 0;

    // /*Save the pressed coordinates and the state*/
    // if(touchpad_is_pressed()) {
    //     touchpad_get_xy(&last_x, &last_y);
    //     data->state = LV_INDEV_STATE_PRESSED;
    // }
    // else {
    //     data->state = LV_INDEV_STATE_RELEASED;
    // }

    // /*Set the last pressed coordinates*/
    // data->point.x = last_x;
    // data->point.y = last_y;

    int16_t x, y;
    int state;
    cst816t_read(&x, &y, &state);
    data->point.x = y;
    if (x == 0)
        x = 1;
    data->point.y = MY_DISP_VER_RES - x;
    data->state = state;
}

/*Return true is the touchpad is pressed*/
static bool touchpad_is_pressed(void)
{
    /*Your code comes here*/

    return false;
}

/*Get the x and y coordinates if the touchpad is pressed*/
static void touchpad_get_xy(int32_t *x, int32_t *y)
{
    /*Your code comes here*/

    (*x) = 0;
    (*y) = 0;
}

esp_err_t lv_port_init(void)
{

    /* 初始化LVGL库 */
    lv_init();
    lvgl_init_display();
    /* 注册显示驱动 */
    lv_port_disp_init();

    // /*触摸芯片初始化*/
    touchpad_init();
    // /* 注册输入驱动*/
    lv_port_indev_init();

    /* 初始化LVGL定时器 */
    lv_port_tick_init();

    return ESP_OK;
}
