/*
 * SPDX-FileCopyrightText: 2021-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <sys/param.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_system.h"
#include "esp_check.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "dns_server.h"

#define DNS_PORT (53)
#define DNS_MAX_LEN (512) // Increased buffer size to accommodate larger packets

#define OPCODE_MASK (0x7800)
#define QR_FLAG (1 << 7)
#define QD_TYPE_A (0x0001)
#define ANS_TTL_SEC (300)

static const char *TAG = "dns_server";

// DNS Header Packet
typedef struct __attribute__((__packed__))
{
    uint16_t id;
    uint16_t flags;
    uint16_t qd_count;
    uint16_t an_count;
    uint16_t ns_count;
    uint16_t ar_count;
} dns_header_t;

// DNS Question Packet
typedef struct
{
    uint16_t type;
    uint16_t class;
} dns_question_t;

// DNS Answer Packet (通用型，适配所有记录类型)
typedef struct __attribute__((__packed__))
{
    uint16_t ptr_offset;
    uint16_t type; // 保留原查询类型
    uint16_t class;
    uint32_t ttl;
    uint16_t addr_len;
    uint32_t ip_addr; // 始终返回指定IP
} dns_answer_t;

// DNS server handle
struct dns_server_handle
{
    bool started;
    TaskHandle_t task;
    int num_of_entries;
    dns_entry_pair_t entry[];
};

/*
    Parse the name from the packet from the DNS name format to a regular .-seperated name
    returns the pointer to the next part of the packet
*/
static char *parse_dns_name(char *raw_name, char *parsed_name, size_t parsed_name_max_len)
{
    // 增加空指针检查（防止崩溃）
    if (raw_name == NULL || parsed_name == NULL || parsed_name_max_len < 2)
    {
        return NULL;
    }

    char *label = raw_name;
    char *name_itr = parsed_name;
    int name_len = 0;

    do
    {
        int sub_name_len = (uint8_t)*label; // 强制无符号，避免负数
        // 检查单段域名长度合法性（DNS规范最大63）
        if (sub_name_len > 63)
        {
            return NULL;
        }
        // (len + 1) since we are adding  a '.'
        name_len += (sub_name_len + 1);
        if (name_len > parsed_name_max_len)
        {
            return NULL;
        }

        // Copy the sub name that follows the the label
        memcpy(name_itr, label + 1, sub_name_len);
        name_itr[sub_name_len] = '.';
        name_itr += (sub_name_len + 1);
        label += sub_name_len + 1;

        // 防止指针越界（最大解析长度限制）
        if ((label - raw_name) > DNS_MAX_LEN)
        {
            return NULL;
        }
    } while (*label != 0);

    // Terminate the final string, replacing the last '.'
    parsed_name[name_len - 1] = '\0';
    // Return pointer to first char after the name
    return label + 1;
}

// Parses the DNS request and prepares a DNS response with the IP of the softAP
// 修改点：移除仅处理A记录的限制，劫持所有类型的DNS查询
static int parse_dns_request(char *req, size_t req_len, char *dns_reply, size_t dns_reply_max_len, dns_server_handle_t h)
{
    if (req_len > dns_reply_max_len || h == NULL)
    {
        return -1;
    }

    // Prepare the reply
    memset(dns_reply, 0, dns_reply_max_len);
    memcpy(dns_reply, req, req_len);

    // Endianness of NW packet different from chip
    dns_header_t *header = (dns_header_t *)dns_reply;
    ESP_LOGD(TAG, "DNS query with header id: 0x%X, flags: 0x%X, qd_count: %d",
             ntohs(header->id), ntohs(header->flags), ntohs(header->qd_count));

    // 【修改1】移除OPCODE检查，处理所有类型的DNS查询（包括反向查询、更新等）
    // if ((header->flags & OPCODE_MASK) != 0) { return 0; }

    // Set question response flag (设置响应标志 + 权威回答 + 无错误)
    header->flags = htons(ntohs(header->flags) | 0x8180); // QR=1, AA=1, RCODE=0

    uint16_t qd_count = ntohs(header->qd_count);
    header->an_count = htons(qd_count); // 回答数 = 查询数

    int reply_len = qd_count * sizeof(dns_answer_t) + req_len;
    if (reply_len > dns_reply_max_len)
    {
        return -1;
    }

    // Pointer to current answer and question
    char *cur_ans_ptr = dns_reply + req_len;
    char *cur_qd_ptr = dns_reply + sizeof(dns_header_t);
    char name[128];

    // Respond to ALL questions (不再限制A记录)
    for (int qd_i = 0; qd_i < qd_count; qd_i++)
    {
        char *name_end_ptr = parse_dns_name(cur_qd_ptr, name, sizeof(name));
        if (name_end_ptr == NULL)
        {
            ESP_LOGE(TAG, "Failed to parse DNS question");
            cur_qd_ptr = cur_qd_ptr + 1; // 移动指针，避免死循环
            continue;                    // 跳过解析失败的查询，处理下一个
        }

        dns_question_t *question = (dns_question_t *)(name_end_ptr);
        uint16_t qd_type = ntohs(question->type);
        uint16_t qd_class = ntohs(question->class);

        ESP_LOGD(TAG, "Received type: %d | Class: %d | Question for: %s", qd_type, qd_class, name);

        // 【修改2】移除A记录限制，处理所有类型的DNS查询
        esp_ip4_addr_t ip = {.addr = IPADDR_ANY};
        // Check the configured rules to decide whether to answer this question or not
        for (int i = 0; i < h->num_of_entries; ++i)
        {
            // check if the name either corresponds to the entry, or if we should answer to all queries ("*")
            if (h->entry[i].name != NULL && (strcmp(h->entry[i].name, "*") == 0 || strcmp(h->entry[i].name, name) == 0))
            {
                if (h->entry[i].if_key)
                {
                    esp_netif_ip_info_t ip_info;
                    esp_netif_t *netif = esp_netif_get_handle_from_ifkey(h->entry[i].if_key);
                    if (netif != NULL && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK)
                    {
                        ip.addr = ip_info.ip.addr;
                    }
                    break;
                }
                else if (h->entry[i].ip.addr != IPADDR_ANY)
                {
                    ip.addr = h->entry[i].ip.addr;
                    break;
                }
            }
        }

        if (ip.addr == IPADDR_ANY)
        { // no rule applies, continue with another question
            cur_qd_ptr = name_end_ptr + sizeof(dns_question_t);
            continue;
        }

        // 为所有类型的查询构造响应（即使不是A记录，也返回指定IP）
        dns_answer_t *answer = (dns_answer_t *)cur_ans_ptr;
        answer->ptr_offset = htons(0xC000 | (cur_qd_ptr - dns_reply)); // 域名指针
        answer->type = htons(qd_type);                                 // 保留原查询类型（关键：兼容非A记录）
        answer->class = htons(qd_class);
        answer->ttl = htonl(ANS_TTL_SEC);
        answer->addr_len = htons(sizeof(ip.addr)); // IP长度固定为4字节
        answer->ip_addr = ip.addr;                 // 返回指定IP

        ESP_LOGD(TAG, "Answer with PTR offset: 0x%" PRIX16 " and IP 0x%" PRIX32, ntohs(answer->ptr_offset), ip.addr);

        cur_ans_ptr += sizeof(dns_answer_t);
        cur_qd_ptr = name_end_ptr + sizeof(dns_question_t);
    }

    // 修正实际的回答长度（处理部分查询解析失败的情况）
    int actual_reply_len = (cur_ans_ptr - dns_reply);
    return actual_reply_len;
}

/*
    Sets up a socket and listen for DNS queries,
    replies to ALL DNS queries with the IP of the softAP
*/
void dns_server_task(void *pvParameters)
{
    char rx_buffer[DNS_MAX_LEN];
    char addr_str[128];
    int addr_family;
    int ip_protocol;
    dns_server_handle_t handle = pvParameters;

    if (handle == NULL)
    {
        ESP_LOGE(TAG, "DNS server task started with NULL handle");
        vTaskDelete(NULL);
        return;
    }

    while (handle->started)
    {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(DNS_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_UDP; // 【明确指定UDP协议】
        inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(1000)); // 失败后延迟重试，避免死循环
            continue;
        }
        ESP_LOGI(TAG, "Socket created");

        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        // 设置接收超时（避免recvfrom永久阻塞）
        struct timeval timeout = {
            .tv_sec = 1,
            .tv_usec = 0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0)
        {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        ESP_LOGI(TAG, "Socket bound, port %d", DNS_PORT);

        while (handle->started)
        {
            struct sockaddr_in6 source_addr;
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0)
            {
                // 超时是正常情况，不报错
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                    break; // 严重错误，重建socket
                }
                continue;
            }
            // Data received
            else
            {
                // Get the sender's ip address as string
                if (source_addr.sin6_family == PF_INET)
                {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                }
                else if (source_addr.sin6_family == PF_INET6)
                {
                    inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                // Null-terminate whatever we received
                rx_buffer[len] = 0;

                char reply[DNS_MAX_LEN];
                int reply_len = parse_dns_request(rx_buffer, len, reply, DNS_MAX_LEN, handle);

                if (reply_len <= 0)
                {
                    ESP_LOGD(TAG, "Failed to prepare a DNS reply for %s", addr_str);
                }
                else
                {
                    int err = sendto(sock, reply, reply_len, 0, (struct sockaddr *)&source_addr, socklen);
                    if (err < 0)
                    {
                        ESP_LOGE(TAG, "Error occurred during sending to %s: errno %d", addr_str, errno);
                        // 发送失败不中断循环，继续处理下一个请求
                        continue;
                    }
                    ESP_LOGD(TAG, "Sent DNS reply to %s (len: %d)", addr_str, reply_len);
                }
            }
        }

        if (sock != -1)
        {
            ESP_LOGI(TAG, "Shutting down socket");
            shutdown(sock, 0);
            close(sock);
        }
    }

    ESP_LOGI(TAG, "DNS server task stopped");
    vTaskDelete(NULL);
}

dns_server_handle_t start_dns_server(dns_server_config_t *config)
{
    // 修复：数组地址永远非空，改为检查num_of_entries是否有效
    if (config == NULL || config->num_of_entries <= 0 || config->num_of_entries > DNS_SERVER_MAX_ITEMS)
    {
        ESP_LOGE(TAG, "Invalid DNS server config (config=%p, num=%d)", config, config ? config->num_of_entries : 0);
        return NULL;
    }

    dns_server_handle_t handle = calloc(1, sizeof(struct dns_server_handle) + config->num_of_entries * sizeof(dns_entry_pair_t));
    ESP_RETURN_ON_FALSE(handle, NULL, TAG, "Failed to allocate dns server handle");

    handle->started = true;
    handle->num_of_entries = config->num_of_entries;
    memcpy(handle->entry, config->item, config->num_of_entries * sizeof(dns_entry_pair_t));

    // 【增大栈空间】从4096改为8192，防止栈溢出
    BaseType_t ret = xTaskCreate(dns_server_task, "dns_server", 8192, handle, 5, &handle->task);
    if (ret != pdPASS)
    {
        ESP_LOGE(TAG, "Failed to create DNS server task");
        free(handle);
        return NULL;
    }

    ESP_LOGI(TAG, "DNS server started with full hijack mode");
    return handle;
}

void stop_dns_server(dns_server_handle_t handle)
{
    if (handle)
    {
        handle->started = false;
        // 等待任务退出（避免直接删除导致内存泄漏）
        if (handle->task != NULL)
        {
            vTaskDelay(pdMS_TO_TICKS(200));
            vTaskDelete(handle->task);
        }
        free(handle);
        ESP_LOGI(TAG, "DNS server stopped");
    }
}