#include "onenet_dm.h"
#include "cJSON.h"
static int s_onenet_id=0;


// ONENET_DM_DES *onenet_malloc_des(void)   //创建物模型结构体
// {
//     ONENET_DM_DES *dm = (ONENET_DM_DES *)malloc(sizeof(ONENET_DM_DES));
//     ONENET_DM_DES *demo = (ONENET_DM_DES *)malloc(sizeof(ONENET_DM_DES));
//     if (dm)
//     {
//         memset(dm, 0, sizeof(ONENET_DM_DES));
//         dm->dm_js = cJSON_CreateObject();
//         if (dm->dm_js == NULL) {
//             free(dm);
//             return NULL;
//         }
        
//         // 添加基础字段
//         char id[10];
//         snprintf(id, sizeof(id), "%d", s_onenet_id);
//         cJSON_AddStringToObject(dm->dm_js, "id", id);
//         cJSON_AddStringToObject(dm->dm_js, "version", "1.0");
        
//         // 创建并添加 params 对象
//         cJSON *params = cJSON_CreateObject();
//         cJSON_AddItemToObject(dm->dm_js, "params", params);
        
//         // 为 params 添加 Power 和 temp 对象
//         demo->dm_js = cJSON_CreateObject();
//         cJSON_AddBoolToObject(demo->dm_js, "value", cJSON_True);
//         // cJSON_AddNumberToObject(LED, "time", 1706673129818);
//         cJSON_AddItemToObject(params, "LED", demo->dm_js);
        
//         // cJSON *temp = cJSON_CreateObject();
//         // cJSON_AddNumberToObject(temp, "value", 23.6);
//         // cJSON_AddNumberToObject(temp, "time", 1706673129818);
//         // cJSON_AddItemToObject(params, "temp", temp);
        
//         return dm;
//     }
//     return NULL;
// }
ONENET_DM_DES *onenet_malloc_des(ONENET_TYPE type)   //创建物模型结构体
{
    ONENET_DM_DES *dm = (ONENET_DM_DES *)malloc(sizeof(ONENET_DM_DES));
    if (dm == NULL) {
        return NULL;
    }
    memset(dm, 0, sizeof(ONENET_DM_DES));
    dm->dm_js = cJSON_CreateObject();
    if (dm->dm_js == NULL) {
        free(dm);
        return NULL;
    }
    
    // 添加基础字段
    char id[10];
    snprintf(id, sizeof(id), "%d", s_onenet_id);
    s_onenet_id++;
    
    cJSON *params = cJSON_CreateObject();
    switch (type)
    {
    case ONENET_POST:
        cJSON_AddStringToObject(dm->dm_js, "id", id);
        cJSON_AddStringToObject(dm->dm_js, "version", "1.0");
        cJSON_AddItemToObject(dm->dm_js, "params", params);
        break;
    case ONENET_SET_ACK:
        cJSON_AddStringToObject(dm->dm_js, "id", id);
        break;
    default:
        break;
    }
    // 创建params对象并添加到根节点
    
    return dm;
}

void onenet_set_dm_int(ONENET_DM_DES* dm, const char* name, int value)  //设置物模型数值属性
{
    if (dm == NULL || name == NULL) {
        return;
    }
    
    // 获取params对象
    cJSON *params = cJSON_GetObjectItem(dm->dm_js, "params");
    if (params == NULL || !cJSON_IsObject(params)) {
        return;
    }
    
    // 创建属性对象
    cJSON *attr_obj = cJSON_CreateObject();
    if (attr_obj == NULL) {
        return;
    }
    
    // 添加value和time字段
    // cJSON_AddNumberToObject(attr_obj, "value", value);
    cJSON_AddBoolToObject(attr_obj, "value", (cJSON_bool)value);
    // cJSON_AddNumberToObject(attr_obj, "time", 1706673129818); // 使用固定时间戳
    
    // 将属性对象添加到params中
    cJSON_AddItemToObject(params, name, attr_obj);
}


void onenet_set_dm_ack(ONENET_DM_DES *dm,int code,const char* msg) //属性回复
{
    if (dm)
    {
        cJSON_AddNumberToObject(dm->dm_js,"code",code); 
        cJSON_AddStringToObject(dm->dm_js,"msg",msg);
    }
}

void onenet_dm_serialize(ONENET_DM_DES *dm) {
    if (dm) {
       if (dm->dm_js_str) {
            free(dm->dm_js_str);  // 使用free释放字符串
            dm->dm_js_str=NULL;
       }
       dm->dm_js_str=cJSON_PrintUnformatted(dm->dm_js);
    }
}

void onenet_dm_free(ONENET_DM_DES *dm)
{
    if (dm)
    {
       if (dm->dm_js_str)
       {
        cJSON_free(dm->dm_js_str);
        dm->dm_js_str=NULL;
       }
       if (dm->dm_js)
       {
        cJSON_Delete(dm->dm_js);
        dm->dm_js=NULL;
       }
       free(dm);
    }
}