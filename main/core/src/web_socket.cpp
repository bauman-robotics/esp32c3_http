#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"

#include "defines.h"
#include "def_pass.h"
#include <math.h>
#include <main.h>

#include <sys/select.h>
#include <sys/time.h>

#include "variables.h"

extern variables var; 

static const char *TAG = "ex";

int calc_sine_socket_data();
static char buf[80] ={0};
static int sin_data_s;

void socket_task(void *pvParameters);
void Pars_Socket_Data(char * rx_buf);

//===============================================================

// void socket_task(void *pvParameters) {
//     #ifdef RECEIVE_ANSWER_ENABLE   
//         char rx_buffer[128];
//     #endif 
//     char addr_str[128];
//     int addr_family;
//     int ip_protocol;

//     while (1) {
//         struct sockaddr_in dest_addr;
//         dest_addr.sin_addr.s_addr = inet_addr(SOCKET_IP);
//         dest_addr.sin_family = AF_INET;
//         dest_addr.sin_port = htons(SOCKET_PORT);
//         addr_family = AF_INET;
//         ip_protocol = IPPROTO_IP;
//         inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

//         int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
//         //int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        
//         if (sock < 0) {
//             ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
//             vTaskDelay(5000 / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
//             continue;  // Продолжить цикл, чтобы попытаться снова
//         }
//         ESP_LOGI(TAG, "Socket created, connecting to %s:%d", SOCKET_IP, SOCKET_PORT);

//         int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//         if (err != 0) {
//             ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
//             close(sock);  // Закрыть сокет перед повторной попыткой
//             vTaskDelay(5000 / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
//             continue;  // Продолжить цикл
//         }
//         ESP_LOGI(TAG, "Successfully connected");

//         while (1) {
//             // Генерация данных для отправки
//             sin_data_s = calc_sine_socket_data();
//             sprintf(buf, "data %d", sin_data_s);

//             int err = send(sock, buf, strlen(buf), 0);       
//             if (err < 0) {
//                 ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//                 break;  // Выход из внутреннего цикла и попытка переподключения
//             } else {
//                 ESP_LOGI(TAG, "send %s", buf);
//             }

//             #ifdef RECEIVE_ANSWER_ENABLE
//                 int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
//                 if (len < 0) {
//                     ESP_LOGE(TAG, "recv failed: errno %d", errno);
//                     //break;  // Выход из внутреннего цикла и попытка переподключения
//                 } 
//                 else if (len == 0) {
//                     ESP_LOGE(TAG, "recv len == 0");
//                 }
//                 else {
//                     rx_buffer[len] = 0;
//                     ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
//                 }
//             #endif 

//             vTaskDelay(SOCKET_SEND_PERIOD_MS / portTICK_PERIOD_MS);
//         }

//         if (sock != -1) {
//             ESP_LOGE(TAG, "Shutting down socket and retrying...");
//             shutdown(sock, 0);
//             close(sock);
//         }

//         vTaskDelay(5000 / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
//     }
// }

//=== асинхронное чтение ===================================================================================

void socket_task(void *pvParameters) {

    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1) {
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(SOCKET_IP);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(SOCKET_PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        //int max_fd = 0;        
        inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        //int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        
        if (sock < 0) {
            var.leds.flags = LEDS_GOT_IP_STATE;            
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            vTaskDelay(5000 / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
            continue;  // Продолжить цикл, чтобы попытаться снова
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", SOCKET_IP, SOCKET_PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            var.leds.flags = LEDS_GOT_IP_STATE;            
            close(sock);  // Закрыть сокет перед повторной попыткой
            vTaskDelay(5000 / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
            continue;  // Продолжить цикл
        }
        ESP_LOGI(TAG, "Successfully connected");

        var.leds.flags = LEDS_CONNECT_TO_SERVER_STATE;
        var.leds.red   = 0;
        var.leds.green = 1;
        var.leds.blue  = 0;

        while (1) {            
            
            //=== Пробуем получить конфигурационные данные =========================
            int num;
            num = recv(sock, rx_buffer, sizeof(rx_buffer), MSG_DONTWAIT);
            if (num == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                // Нет данных для чтения, но сокет остается активным
                // Можно выполнить другие действия и попробовать снова позже
                //ESP_LOGE(TAG, "no data yet");
            } else if (num > 0) {
                // Обработка входящих данных
                rx_buffer[num] = '\0';  // Завершающий символ строки
                Pars_Socket_Data(rx_buffer);
                //ESP_LOGI(TAG, "_____________________________________Received %d bytes from server: %s", num, rx_buffer);
            }
            //======================================================================

            //=== Генерация данных для отправки ====================================
            sin_data_s = calc_sine_socket_data();
            sprintf(buf, "data %d", sin_data_s);

            //int err = send(sock, buf, strlen(buf), 0);       
            int err = send(sock, buf, strlen(buf), MSG_NOSIGNAL); 
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;  // Выход из внутреннего цикла и попытка переподключения
            } else {
                ESP_LOGI(TAG, "> %s", buf);
            }

            vTaskDelay(SOCKET_SEND_PERIOD_MS / portTICK_PERIOD_MS);
            //======================================================================
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and retrying...");
            shutdown(sock, 0);
            close(sock);
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
    }
}
//==============================================================================================================

// Определение флагов
#define FLAG1 0x01
#define FLAG2 0x02
#define FLAG3 0x04

// Функция анализа строки и установки флагов
void Pars_Socket_Data(char *rx_buf) {
    // Инициализация флагов
    unsigned char flags = 0x00;

    // Определение вариантов соответствия
    const char *options[] = {"Red", "Green", "Blue"};

    // Анализ строки и установка флагов
    for (int i = 0; i < 3; i++) {
        if (strstr(rx_buf, options[i]) != NULL) {
            switch (i) {
                case 0:
                    flags |= FLAG1;
                    ESP_LOGI(TAG, "________________________Red");

                    var.leds.red   = 1;
                    var.leds.green = 0;
                    var.leds.blue  = 0;
                    break;
                //=====================    
                case 1:
                    flags |= FLAG2;
                    ESP_LOGI(TAG, "________________________Green");

                    var.leds.red   = 0;
                    var.leds.green = 1;
                    var.leds.blue  = 0;
                    break;
                //===================== 
                case 2:
                    flags |= FLAG3;
                    ESP_LOGI(TAG, "________________________Blue");

                    var.leds.red   = 0;
                    var.leds.green = 0;
                    var.leds.blue  = 1;                    
                    break;
            }
        }
    }

    // Вывод флагов
    ESP_LOGI(TAG, "Флаги: 0x%02X\n", flags);
}