#pragma once

#include "esp_http_client.h"

esp_err_t _http_event_handler(esp_http_client_event_t *evt);
void send_post_request(int16_t cold, int16_t hot, int16_t alarm_interval);
void ping_test(const char* target);
void wifi_init_sta(void); 
 