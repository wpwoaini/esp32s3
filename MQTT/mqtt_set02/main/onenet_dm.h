#ifndef _ONENET_DM_H_
#define _ONENET_DM_H_
#include "cJSON.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "mqtt_al.h"
typedef enum
{
    ONENET_POST,
    ONENET_SET_ACK,
} ONENET_TYPE;

typedef struct
{
    cJSON *dm_js;
    char *dm_js_str;
} ONENET_DM_DES;

ONENET_DM_DES *onenet_malloc_des(ONENET_TYPE type); // 创建物模型结构体

void onenet_set_dm_int(ONENET_DM_DES *dm, const char *name, int value); // 设置物模型属性
void onenet_dm_serialize(ONENET_DM_DES *dm);                            // json字符串

void onenet_dm_free(ONENET_DM_DES *dm);

void onenet_set_dm_ack(ONENET_DM_DES *dm, int code, const char *msg); // 属性回复

#if 0
ONENET_DM_DES *dm = onenet_malloc_des();
onenet_set_dm_int(dm,"LED",1);
onenet_dm_serialize(dm);
dm->dm_js_str;

#endif

#endif