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

#include "pars_cmd.h"
#include "make_data_str.h" 

extern void ina226_init_task(void *pvParameters);

static const char *TAG = "ex";

void socket_task(void *pvParameters);

int Add_Number_To_String(char *str, int number, const char *prefix, int *element_count, int max_elements); 
int Add_Number_To_String_Float(char *str, float number_f, const char *prefix, int *element_count, int max_elements); 

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
            var.mode.flags = LEDS_GOT_IP_STATE;            
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            vTaskDelay(5000 / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
            continue;  // Продолжить цикл, чтобы попытаться снова
        }
        ESP_LOGI(TAG, "Socket created, connecting to %s:%d", SOCKET_IP, SOCKET_PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
            var.mode.flags = LEDS_GOT_IP_STATE;            
            close(sock);  // Закрыть сокет перед повторной попыткой
            vTaskDelay(5000 / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
            continue;  // Продолжить цикл
        }
        ESP_LOGI(TAG, "Successfully connected");

        var.mode.flags = LEDS_CONNECT_TO_SERVER_STATE;

        var.mode.saw   = 0;
        var.mode.sin   = 1;
        var.mode.ina226  = 0;

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
                Pars_Cmd(rx_buffer);
                //ESP_LOGI(TAG, "_____________________________________Received %d bytes from server: %s", num, rx_buffer);
            }
            //======================================================================

            //=== Генерация данных для отправки ====================================
            uint8_t ready_flag = 0;
            // Получение флага готовности из очереди
            if (xQueueReceive(xQueueSignalReady, &ready_flag, portMAX_DELAY)) {
                if (ready_flag == 1) {
                    //ESP_LOGI(TAG, "ready_flag Received ");        
                    #ifdef DEBUG_LOG
                        ESP_LOGI(TAG, "Data is ready!");
                    #endif 

                    // Получение данных из очереди
                    SignalData signal_data;
                    if (xQueueReceive(xQueueSignalData, &signal_data, portMAX_DELAY)) {
                        //ESP_LOGI(TAG, "Received signal data:");
                        #ifdef DEBUG_LOG
                            ESP_LOGI(TAG, "Received signal data:");
                            ESP_LOGI(TAG, "Receive_Q_Signal data[0]=%d", signal_data.data[0]);
                            ESP_LOGI(TAG, "Receive_Q_Signal data[1]=%d", signal_data.data[1]); 
                        #endif 

    
                        int err = 0;
                        if (!var.packet.type_hex) {
                            //=== Заполнили стороку пакета данными ===
                            for (int i = 0; i < var.count_vals_in_packet; i++) {
                                
                                //Add_Number_To_String(var.packet.buf, signal_data.data[i], DATA_PREFIX_INT, &var.packet.count_el, var.count_vals_in_packet); 
                                #ifndef DATA_TYPE_FLOAT    
                                    Add_Number_To_String(var.packet.buf, signal_data.data[i], DATA_PREFIX_INT, &var.packet.count_el, var.count_vals_in_packet); 
                                #else 
                                    Add_Number_To_String_Float(var.packet.buf, signal_data.data_f[i], DATA_PREFIX_FLOAT, &var.packet.count_el, var.count_vals_in_packet); 
                                #endif
                                //ESP_LOGI(TAG, "Data[%d]: %d", i, signal_data.data[i]);
                            }

                            //=== Отправка пакета ====
                            err = send(sock, var.packet.buf, strlen(var.packet.buf), MSG_NOSIGNAL); 
                        }
                        else {
                            //err = send(sock, &signal_data, signal_data.header.full_packet_size, MSG_NOSIGNAL);      
                            #ifndef DATA_TYPE_FLOAT 
                                err = send(sock, &signal_data, signal_data.header_int.full_packet_size, MSG_NOSIGNAL);    
                            #else 
                                err = send(sock, &signal_data, signal_data.header_float.full_packet_size, MSG_NOSIGNAL);  
                            #endif              
                        } 

                        if (err < 0) {
                            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                            break;  // Выход из внутреннего цикла и попытка переподключения
                        } else {
                            
                            #ifdef DEBUG_LOG
                                ESP_LOGI(TAG, "> signal_data header.type %d ", signal_data.header.type);
                                ESP_LOGI(TAG, "> signal_data header.full_packet_size %d ", signal_data.header.full_packet_size);
                                ESP_LOGI(TAG, "> signal_data data[0] %d ", signal_data.data[0]);
                                ESP_LOGI(TAG, "> signal_data data[1] %d ", signal_data.data[1]);
                                ESP_LOGI(TAG, "> Send %d  bytes ", signal_data.header.full_packet_size);
                            #endif 
                        }
                        var.packet.buf[0] = 0;
                        var.packet.count_el = 0;    

                    } else {
                        ESP_LOGI(TAG, "Failed to receive signal data!");
                    }
                }
            } else {
                ESP_LOGI(TAG, "Failed to receive ready flag!");
            }     

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


