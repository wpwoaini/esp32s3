#ifndef _MQTT_AL_H_
#define _MQTT_AL_H_

#include "mqtt_client.h"

#define DEVICENAME "cam"                                            // 设备名称
#define USERNAME "3OYc99Dt2k"                                       // 用户名称
#define PRODUCTKEY ""                                               // 产品密钥
#define DEVICESECRET "bkY0UHhQdUNsU29RVTdoR2JMbnd6RVRpdDVYdTB4c2U=" // 设备密钥
#define MQTT_URL "mqtts://mqttstls.heclouds.com"                    // MQTT域名
#define MQTT_PORT 8883                                              // 端口
#define TOKEN "version=2018-10-31&res=products%2F3OYc99Dt2k%2Fdevices%2Fcam&et=1832935736&method=md5&sign=UyvuLmGLjR5%2BwxqrhhhU%2FA%3D%3D"
// products/3OYc99Dt2k/devices/cam
// 设备属性上报请求
#define MQTT_POST "$sys/3OYc99Dt2k/cam/thing/property/post"
// 设备属性上报响应
#define MQTT_POST_REPLY "$sys/3OYc99Dt2k/cam/thing/property/post/reply"
// 设备属性设置请求
#define MQTT_SET "$sys/3OYc99Dt2k/cam/thing/property/set"
// 设备属性设置响应
#define MQTT_SET_REPLY "$sys/3OYc99Dt2k/cam/thing/property/set_reply"

extern esp_mqtt_client_handle_t s_mqtt_client;

// 启动mqtt
void mqtt_start(void);

// 是否连接
char is_mqtt_connected(void);

// 上报int数据
void onenet_post_int(const char *name, int value);

// 上报属性回复
void onenet_ack(int code, const char *msg);
void send_custom_ack(int code, const char *msg);
#endif
