#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "dns_server.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "wifi_manage.h"
static const char *TAG = "MAIN";

void wifi_mange_cb(wifi_manage_event_t event)
{
    switch (event)
    {
    case WIFI_MANAGE_EVENT_STA_GOT_IP:
        /* code */
        break;

    default:
        break;
    }
}

void app_main(void)
{
    nvs_flash_init();
    wifi_manage_init(wifi_mange_cb);
    wifi_manage_start();
}