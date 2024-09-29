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
#include <math.h>
#include <main.h>

static const char *TAG = "socket_example";

int calc_sine_socket_data();
static char buf[80] ={0};
static int sin_data_s;

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
//         if (sock < 0) {
//             ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
//             break;
//         }
//         ESP_LOGI(TAG, "Socket created, connecting to %s:%d", SOCKET_IP, SOCKET_PORT);

//         int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//         if (err != 0) {
//             ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
//             close(sock);
//             break;
//         }
//         ESP_LOGI(TAG, "Successfully connected");

//         while (1) {
//             //int err = send(sock, "GET / HTTP/1.1\r\n\r\n", 18, 0);

            
//             sin_data_s = calc_sine_socket_data();
//             sprintf(buf, "data %d", sin_data_s);
//             //sprintf(buf, "data %d%s", sin_data_s,"\n");


//             // if (sin_data_s < 1000) sin_data_s = sin_data_s + 2;
//             // else sin_data_s = 0;
//             // sprintf(buf, "data %d%s%d" , sin_data_s, " data " , sin_data_s+1);


//             int err = send(sock, buf, strlen(buf), 0);       
                 
//             if (err < 0) {
//                 ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//                 break;
//             }
//             #ifdef RECEIVE_ANSWER_ENABLE
//                 int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
//                 if (len < 0) {
//                     ESP_LOGE(TAG, "recv failed: errno %d", errno);
//                     break;
//                 } else {
//                     rx_buffer[len] = 0;
//                     ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
//                 }
//             #endif 
//             vTaskDelay(SOCKET_SEND_PERIOD_MS / portTICK_PERIOD_MS);
//         }

//         if (sock != -1) {
//             ESP_LOGE(TAG, "Shutting down socket and restarting...");
//             shutdown(sock, 0);
//             close(sock);
//         }

//         vTaskDelay(5000 / portTICK_PERIOD_MS);
//     }
// }

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
//         if (sock < 0) {
//             ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
//             vTaskDelay(5000 / portTICK_PERIOD_MS); // Пауза перед повторной попыткой создания сокета
//             continue; // Переходим к следующей итерации цикла
//         }
//         ESP_LOGI(TAG, "Socket created, connecting to %s:%d", SOCKET_IP, SOCKET_PORT);

//         int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//         if (err != 0) {
//             ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
//             close(sock);
//             vTaskDelay(5000 / portTICK_PERIOD_MS); // Пауза перед повторной попыткой подключения
//             continue; // Переходим к следующей итерации цикла
//         }
//         ESP_LOGI(TAG, "Successfully connected");

//         while (1) {
//             // ... ваш код для отправки и получения данных ...

//             if (err < 0) {
//                 ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//                 break; // Выходим из внутреннего цикла при ошибке отправки
//             }
//             #ifdef RECEIVE_ANSWER_ENABLE
//                 int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
//                 if (len < 0) {
//                     ESP_LOGE(TAG, "recv failed: errno %d", errno);
//                     break; // Выходим из внутреннего цикла при ошибке получения
//                 } else {
//                     // ... ваш код для обработки полученных данных ...
//                 }
//             #endif 
//             vTaskDelay(SOCKET_SEND_PERIOD_MS / portTICK_PERIOD_MS);
//         }

//         if (sock != -1) {
//             ESP_LOGE(TAG, "Shutting down socket and restarting...");
//             shutdown(sock, 0);
//             close(sock);
//         }

//         vTaskDelay(5000 / portTICK_PERIOD_MS); // Пауза перед повторной попыткой подключения
//     }
// }

//===============================================================

void socket_task(void *pvParameters) {
    #ifdef RECEIVE_ANSWER_ENABLE   
        char rx_buffer[128];
    #endif 
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
        inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        //int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            vTaskDelay(5000 / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
            continue;  // Продолжить цикл, чтобы попытаться снова
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", SOCKET_IP, SOCKET_PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            close(sock);  // Закрыть сокет перед повторной попыткой
            vTaskDelay(5000 / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
            continue;  // Продолжить цикл
        }
        ESP_LOGI(TAG, "Successfully connected");

        while (1) {
            // Генерация данных для отправки
            sin_data_s = calc_sine_socket_data();
            sprintf(buf, "data %d", sin_data_s);

            int err = send(sock, buf, strlen(buf), 0);       
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;  // Выход из внутреннего цикла и попытка переподключения
            } else {
                ESP_LOGI(TAG, "send %s", buf);
            }

            #ifdef RECEIVE_ANSWER_ENABLE
                int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
                if (len < 0) {
                    ESP_LOGE(TAG, "recv failed: errno %d", errno);
                    break;  // Выход из внутреннего цикла и попытка переподключения
                } else {
                    rx_buffer[len] = 0;
                    ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
                }
            #endif 

            vTaskDelay(SOCKET_SEND_PERIOD_MS / portTICK_PERIOD_MS);
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and retrying...");
            shutdown(sock, 0);
            close(sock);
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
    }
}