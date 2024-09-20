#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_system.h"
#include "esp_http_client.h"
//============
#include "ping/ping_sock.h"
#include "lwip/inet.h"
#include "lwip/err.h"
#include "lwip/sys.h"
//============
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//============
#include <string>
#include <stdio.h>
//============
#include "defines.h"
#include "def_pass.h"
#include "http_func.h"
//============

