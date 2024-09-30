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
#include "signal_gen.h"


static const char *TAG = "ex";


static char buf[80] ={0};
static int signal_forms;

void socket_task(void *pvParameters);
void Pars_Socket_Data(char * rx_buf);
int Add_Number_To_String(char *str, int number, const char *prefix, int *element_count, int max_elements); 

//=== Отправка пакетов и Асинхронное чтение ===================================================================================

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
            

            // if (var.leds.green) {
            //     signal_forms = (int)calc_sine_socket_data();
            // }
            // else if (var.leds.red) {
            //     signal_forms = (int)calc_sawtooth_socket_data();
            // }
            // else if (var.leds.blue) {
            //     signal_forms = (int)calc_triangle_socket_data();
            // }

            // Получение значения из очереди
            if (xQueueReceive(xQueue, &signal_forms, portMAX_DELAY) == pdPASS) {
                //ESP_LOGI(TAG, "Received value from Queue: %d\n", signal_forms);
            } else {
                ESP_LOGE(TAG, "Failed to receive value from Queue!");
            }
            int is_buf_full = Add_Number_To_String(var.packet.buf, signal_forms, DATA_PREFIX, &var.packet.count_el, NUM_ELEMENT_IN_PACKET); 

            if (is_buf_full) {
                int err = send(sock, var.packet.buf, strlen(var.packet.buf), MSG_NOSIGNAL); 
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;  // Выход из внутреннего цикла и попытка переподключения
                } else {
                    ESP_LOGI(TAG, "> %s", var.packet.buf);
                }
                var.packet.buf[0] = 0;
                var.packet.count_el = 0;                
            }
            
            //=== was work ===
            //sprintf(buf, "data %d", signal_forms);    
            // int err = send(sock, buf, strlen(buf), MSG_NOSIGNAL); 
            // if (err < 0) {
            //     ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            //     break;  // Выход из внутреннего цикла и попытка переподключения
            // } else {
            //     ESP_LOGI(TAG, "> %s", buf);
            // }
            //================

            vTaskDelay(var.signal_period / portTICK_PERIOD_MS);
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

// Функция анализа строки и установки флагов
void Pars_Socket_Data(char *rx_buf) {

    // Определение вариантов соответствия
    const char *options[] = {"Red", "Green", "Blue"};

    // Анализ строки и установка флагов
    for (int i = 0; i < 3; i++) {
        if (strstr(rx_buf, options[i]) != NULL) {
            switch (i) {
                case 0:
                    ESP_LOGI(TAG, "________________________Red");

                    var.leds.red   = 1;
                    var.leds.green = 0;
                    var.leds.blue  = 0;
                    break;
                //=====================    
                case 1:
                    ESP_LOGI(TAG, "________________________Green");

                    var.leds.red   = 0;
                    var.leds.green = 1;
                    var.leds.blue  = 0;
                    break;
                //===================== 
                case 2:
                    ESP_LOGI(TAG, "________________________Blue");

                    var.leds.red   = 0;
                    var.leds.green = 0;
                    var.leds.blue  = 1;                    
                    break;
            }
        }
    }

    // Дополнительный код для обработки ключевого слова "PER"
    char *per_ptr = strstr(rx_buf, "PER");
    if (per_ptr != NULL) {
        per_ptr += 3; // Переходим за "PER"

        // Пропускаем пробелы и недопустимые символы
        while (*per_ptr != '\0' && !isdigit((unsigned char)*per_ptr)) {
            per_ptr++;
        }

        if (*per_ptr != '\0') {
            // Извлекаем целое значение после "PER"
            int signal_period = atoi(per_ptr);
            ESP_LOGI(TAG, "________________________PER value: %d", signal_period);

            var.signal_period = signal_period;
        } else {
            ESP_LOGI(TAG, "________________________No value found after PER");
        }
    }

}
//==============================================================================================================


// Функция для добавления числа в строку и проверки заполненности
int Add_Number_To_String(char *str, int number, const char *prefix, int *element_count, int max_elements) {
    char number_str[12]; // Буфер для хранения строкового представления числа
    sprintf(number_str, "%d%s", number, " "); // Конвертация числа в строку

    // Добавляем префикс и число
    strcat(str, prefix);
    strcat(str, number_str);

    // Увеличиваем счетчик количества элементов
    (*element_count)++;

    //ESP_LOGI(TAG, "Add data to string %d", number);

    // Проверяем, достигли ли мы максимального количества элементов
    if (*element_count >= max_elements) {
        //ESP_LOGI(TAG, "Packet full");
        return 1; // Строка заполнена        
    }

    return 0; // Строка еще не заполнена
}