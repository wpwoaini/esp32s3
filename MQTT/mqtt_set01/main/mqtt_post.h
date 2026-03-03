#ifndef __MQTT_POST_H__
#define __MQTT_POST_H__

#include "mqtt_client.h"
#include "cJSON.h"
#include <stdbool.h>
#include <stdint.h>
#include "onenet_dm.h"
esp_err_t mqtt_publish_cam_struct_onenet(esp_mqtt_client_handle_t client,
                                         const char *topic,
                                         const char *id,
                                         const char *cam1_val,
                                         const char *cam2_val,
                                         const char *cam3_val,
                                         const char *cam4_val,
                                         const char *cam5_val);
#endif