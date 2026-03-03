#include "onenet_dm.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "led_ws2812.h"
#include <string.h>

// 灯的开关状态
static int led_status = 0;

// 保存当前灯的亮度
static int led_brightness = 0;

// 保存WS2812的RGB值
static uint8_t ws2812_red = 0;
static uint8_t ws2812_green = 0;
static uint8_t ws2812_blue = 0;

// ws2812操作句柄
static ws2812_strip_handle_t ws2812_handle;

/**
 * 物模型数据初始化
 * @param 无
 * @return 无
 */
void onenet_dm_init(void)
{
    // 初始化ws2812
    ws2812_init(GPIO_NUM_48, 1, &ws2812_handle);
    // LED初始化
    ledc_timer_config_t led_timer =
        {
            .clk_cfg = LEDC_AUTO_CLK,
            .duty_resolution = LEDC_TIMER_12_BIT, // 2^12-1
            .freq_hz = 5000,
            .timer_num = LEDC_TIMER_0,
        };
    ledc_timer_config(&led_timer);
    // PWM通道初始化
    ledc_channel_config_t led_channel =
        {
            .channel = LEDC_CHANNEL_0,
            .duty = 0,
            .gpio_num = GPIO_NUM_15,
            .timer_sel = LEDC_TIMER_0,
        };
    ledc_channel_config(&led_channel);
    // 启用渐变功能，如果要调节占空比，必须启用这句
    ledc_fade_func_install(0);
}

/**
 * 处理onenet下行的数据
 * @param property_js 包含下行数据的json
 * @return 无
 */
void onenet_property_handle(cJSON *property_js)
{
    cJSON *params_js = cJSON_GetObjectItem(property_js, "params");
    if (params_js)
    {
        cJSON *name_js = params_js->child;
        while (name_js)
        {
            if (strcmp(name_js->string, "LightSwitch") == 0) // 开关数据
            {
                if (cJSON_IsTrue(name_js))
                {
                    led_brightness = 50;
                    led_status = 1;
                    int duty = 50 * 4095 / 100;
                    ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty, 0);
                }
                else
                {
                    ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 0);
                    led_status = 0;
                    led_brightness = 0;
                }
            }
            else if (strcmp(name_js->string, "Brightness") == 0) // 亮度数据
            {
                led_brightness = cJSON_GetNumberValue(name_js);
                int duty = led_brightness * 4095 / 100;
                ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty, 0);
            }
            else if (strcmp(name_js->string, "RGBColor") == 0) // RGB数据
            {
                ws2812_red = cJSON_GetNumberValue(cJSON_GetObjectItem(name_js, "Red"));
                ws2812_green = cJSON_GetNumberValue(cJSON_GetObjectItem(name_js, "Green"));
                ws2812_blue = cJSON_GetNumberValue(cJSON_GetObjectItem(name_js, "Blue"));
                for (int i = 0; i < 3; i++)
                {
                    ws2812_write(ws2812_handle, i, ws2812_red, ws2812_green, ws2812_blue);
                }
            }
            name_js = name_js->child;
        }
    }
}

/**
 * 生成上报所有数据的cJSON对象
 * @param 无
 * @return cJSON对象，包含所有属性值
 */
cJSON *onenet_property_upload_dm(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "id", "123");
    cJSON_AddStringToObject(root, "version", "1.0");
    cJSON *params_js = cJSON_AddObjectToObject(root, "params");
    // 灯开关
    cJSON *light_js = cJSON_AddObjectToObject(params_js, "LightSwitch");
    cJSON_AddBoolToObject(light_js, "value", led_status);
    // 亮度
    cJSON *brightness_js = cJSON_AddObjectToObject(params_js, "Brightness");
    cJSON_AddNumberToObject(brightness_js, "value", led_brightness);
    // RGB
    cJSON *color_js = cJSON_AddObjectToObject(params_js, "RGBColor");
    cJSON *color_value_js = cJSON_AddObjectToObject(color_js, "value");
    cJSON_AddNumberToObject(color_value_js, "Red", ws2812_red);
    cJSON_AddNumberToObject(color_value_js, "Green", ws2812_green);
    cJSON_AddNumberToObject(color_value_js, "Blue", ws2812_blue);
    return root;
}
