#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "wifi_manager.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include "onenet_mqtt.h"
#include "onenet_dm.h"

#define TAG "main"

#define EV_WIFI_CONNECTED_BIT (1 << 0)

#define DEFAULT_SSID "Xiaomi_User"
#define DEFAULT_PASSWORD "996633aa"

static EventGroupHandle_t s_wifi_ev;

/**
 * wifi事件回调函数
 * @param ev wifi事件
 * @return 无
 */
void wifi_callback(WIFI_STATE ev)
{
    if (ev == WIFI_STATE_CONNECTED)
    {
        xEventGroupSetBits(s_wifi_ev, EV_WIFI_CONNECTED_BIT);
    }
}

void app_main(void)
{
    // nvs初始化
    nvs_flash_init();
    // onenet物模型数据初始化
    onenet_dm_init();
    s_wifi_ev = xEventGroupCreate();
    wifi_manager_init(wifi_callback);
    wifi_manager_connect(DEFAULT_SSID, DEFAULT_PASSWORD);

    while (1)
    {
        // 这里等待一个WIFI连接成功的事件，然后再启动onenet连接
        EventBits_t bits = xEventGroupWaitBits(s_wifi_ev, EV_WIFI_CONNECTED_BIT, pdTRUE, pdFALSE, pdMS_TO_TICKS(5000));
        if (bits & EV_WIFI_CONNECTED_BIT)
        {
            onenet_start();
        }
    }
}
