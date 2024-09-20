#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
//============
#include "ping/ping_sock.h"
#include "lwip/inet.h"
#include "esp_tls.h"
//============
#include <string>
//============
#include "defines.h"
#include "def_pass.h"
#include "http_func.h"
//============

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "lwip/err.h"
#include "lwip/sys.h"
//============

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;
#define MAX_RETRY 5
//======================================================================
#define SERIAL_PRINT_DEBUG_EN (1)

extern const char *TAG;

// Replace with your network credentials
const char* ssid       = SSID; 
const char* password   = WIFI_PAS; 
const char* serverName = SERVER_NAME;

// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 

std::string user_id       = "Andrey";
std::string user_location = "Home";
std::string apiKeyValue   = API_KEY;

char buf_cold[80];
char buf_hot[80];

unsigned long wakeUpTime;
time_t now; 
struct tm timeinfo;
//=====================================================================================

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch((int)evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (!esp_http_client_is_chunked_response(evt->client)) {
                printf("%.*s", evt->data_len, (char*)evt->data);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            break;
    }
    return ESP_OK;
}
//=====================================================================================

void send_post_request(int cold, int hot, int alarm_interval) {
    esp_http_client_config_t config = {
        .url = serverName,
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Установите таймаут для соединения
    esp_err_t err = esp_http_client_set_timeout_ms(client, 5000);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set timeout: %s", esp_err_to_name(err));
        return;
    }

    char post_data[256];
    snprintf(post_data, sizeof(post_data), "api_key=%s&user_id=%s&user_location=%s&cold=%d&hot=%d&alarm_time=%d", 
             apiKeyValue.c_str(), user_id.c_str(), user_location.c_str(), cold, hot, alarm_interval);

    ESP_LOGI(TAG, "Sending HTTP POST request to %s", serverName);
    ESP_LOGI(TAG, "POST data: %s", post_data);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(client),
                 (long long int)esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}
//=====================================================================================

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
//=====================================================================================

void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config)); // Обнуление структуры
    strcpy(reinterpret_cast<char*>(wifi_config.sta.ssid), SSID);
    strcpy(reinterpret_cast<char*>(wifi_config.sta.password), WIFI_PAS);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // Исправлено: WIFI_IF_STA вместо ESP_IF_WIFI_STA
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", wifi_config.sta.ssid, wifi_config.sta.password);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", wifi_config.sta.ssid, wifi_config.sta.password);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}
//=====================================================================================

// void wifi_init_sta(void) {
//     esp_netif_init();
//     esp_event_loop_create_default();
//     esp_netif_create_default_wifi_sta();
//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     esp_wifi_init(&cfg);
//     esp_wifi_set_mode(WIFI_MODE_STA);
//     wifi_config_t wifi_config = {};
//     strcpy((char*)wifi_config.sta.ssid, ssid);
//     strcpy((char*)wifi_config.sta.password, password);
//     esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
//     esp_wifi_start();
//     esp_wifi_connect();
// }


//=====================================================================================




void ping_test(const char* target) {
    ip4_addr_t target_addr;
    inet_aton(target, &target_addr); // Преобразует строку в IP-адрес и сохраняет его в структуре

    ESP_LOGI(TAG, "Pinging target: %s", target);

    esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();
    config.target_addr.u_addr.ip4 = target_addr;
    config.target_addr.type = IPADDR_TYPE_V4;

    // Логируем все параметры ping
    ESP_LOGI(TAG, "Ping Configuration:");
    ESP_LOGI(TAG, "Target Address: %s", ip4addr_ntoa(&target_addr));
    ESP_LOGI(TAG, "Count: %d", (unsigned int)config.count);
    ESP_LOGI(TAG, "Interval: %d ms", (unsigned int)config.interval_ms);
    ESP_LOGI(TAG, "Timeout: %d ms", (unsigned int)config.timeout_ms);
    ESP_LOGI(TAG, "Data size: %d bytes", (unsigned int)config.data_size);
    ESP_LOGI(TAG, "TTL: %d", (unsigned int)config.ttl);

    esp_ping_callbacks_t cbs = {
        .cb_args = NULL,
        .on_ping_success = [](esp_ping_handle_t hdl, void *args) {
            uint8_t ttl;
            uint16_t seqno;
            uint32_t elapsed_time, recv_len;
            ip_addr_t target_addr;
            esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
            esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
            esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
            esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
            esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
            ESP_LOGI(TAG, "Ping success: %u bytes from %s icmp_seq=%u ttl=%u time=%u ms",
                     (unsigned int)recv_len, ip4addr_ntoa(&target_addr.u_addr.ip4), (unsigned int)seqno, (unsigned int)ttl, (unsigned int)elapsed_time);
        },
        .on_ping_timeout = [](esp_ping_handle_t hdl, void *args) {
            uint16_t seqno;
            ip_addr_t target_addr;
            esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
            esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
            ESP_LOGW(TAG, "Ping timeout: From %s icmp_seq=%u", ip4addr_ntoa(&target_addr.u_addr.ip4), (unsigned int)seqno);
        },
        .on_ping_end = [](esp_ping_handle_t hdl, void *args) {
            ESP_LOGI(TAG, "Ping test finished");
            esp_ping_delete_session(hdl);
        }
    };

    esp_ping_handle_t ping;
    esp_ping_new_session(&config, &cbs, &ping);
    esp_ping_start(ping);
}
//=====================================================================================


// void wait_for_ip() {
//     esp_netif_ip_info_t ip_info;
//     esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
//     if (netif) {
//         while (true) {
//             esp_netif_get_ip_info(netif, &ip_info);
//             if (ip_info.ip.addr != IPADDR_ANY) {
//                 ESP_LOGI(TAG, "Got IP Address: " IPSTR, IP2STR(&ip_info.ip));
//                 break;
//             }
//             ESP_LOGI(TAG, "Waiting for IP...");
//             vTaskDelay(1000 / portTICK_PERIOD_MS);
//         }
//     } else {
//         ESP_LOGE(TAG, "Failed to get network interface handle");
//     }
// }