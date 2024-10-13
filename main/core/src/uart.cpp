#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/usb_serial_jtag.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_check.h"

#include <stdio.h>
#include <string.h>

#include "variables.h"
//#include "signal_gen.h"
#include "pars_cmd.h"
#include "make_data_str.h" 

#define BUF_SIZE (1024)

static const char *TAG = "uart";


void usb_serial_jtag_task(void *arg)
{

    usb_serial_jtag_driver_config_t usb_serial_jtag_config;
    usb_serial_jtag_config.rx_buffer_size = BUF_SIZE;
    usb_serial_jtag_config.tx_buffer_size = BUF_SIZE;    

    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb_serial_jtag_config));
    ESP_LOGI("usb_serial_jtag echo", "USB_SERIAL_JTAG init done");

    // Configure a temporary buffer for the incoming data
    //uint8_t *data = (uint8_t *) malloc(BUF_SIZE);  // not work  correctly Pars_Cmd(data);     
    char *data = (char *) malloc(BUF_SIZE);    
    if (data == NULL) {
        ESP_LOGE("usb_serial_jtag echo", "no memory for data");
        return;
    }

    while (1) {

        int num = usb_serial_jtag_read_bytes(data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);

        //=== Пробуем получить конфигурационные данные =========================
        if (num > 0) {
            // Обработка входящих данных
            data[num] = '\0';  // Завершающий символ строки

            Pars_Cmd(data);               
            //ESP_LOGI(TAG, "_____________________________________Received %d bytes from server: %s", num, data);        
        }
        //======================================================================

        //=== Генерация данных для отправки ====================================
        uint8_t ready_flag = 0;
        //Получение флага готовности из очереди
        if (xQueueReceive(xQueueSignalReady, &ready_flag, portMAX_DELAY)) {
            if (ready_flag == 1) {                 
                //ESP_LOGI(TAG, "ready_flag Received ");        
                // #ifdef DEBUG_LOG
                //     ESP_LOGI(TAG, "Data is ready!");
                // #endif 

                // Получение данных из очереди
                SignalData signal_data;
                if (xQueueReceive(xQueueSignalData, &signal_data, portMAX_DELAY)) {
                    //ESP_LOGI(TAG, "Received signal data:");
                    #ifdef DEBUG_LOG
                        ESP_LOGI(TAG, "Received signal data:");
                        ESP_LOGI(TAG, "Receive_Q_Signal data[0]=%d", signal_data.data[0]);
                        ESP_LOGI(TAG, "Receive_Q_Signal data[1]=%d", signal_data.data[1]); 
                    #endif 
                    
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
                        //err = send(sock, var.packet.buf, strlen(var.packet.buf), MSG_NOSIGNAL); 

                        int res;
                        res = usb_serial_jtag_write_bytes(var.packet.buf, strlen(var.packet.buf), 20 / portTICK_PERIOD_MS);
                        //printf("%s\n", var.packet.buf);
                    }
                    else {
                        //err = send(sock, &signal_data, signal_data.header.full_packet_size, MSG_NOSIGNAL);      
                        #ifndef DATA_TYPE_FLOAT 
                            //err = send(sock, &signal_data, signal_data.header_int.full_packet_size, MSG_NOSIGNAL); 
                            //printf("%s\n", signal_data);   
                        #else 

                            usb_serial_jtag_write_bytes(&signal_data, signal_data.header_float.full_packet_size, 20 / portTICK_PERIOD_MS);

                            //err = send(sock, &signal_data, signal_data.header_float.full_packet_size, MSG_NOSIGNAL);  
                            //printf("%s\n", signal_data); 
                        #endif              
                    } 
                        
                    #ifdef DEBUG_LOG
                        ESP_LOGI(TAG, "> signal_data header.type %d ", signal_data.header.type);
                        ESP_LOGI(TAG, "> signal_data header.full_packet_size %d ", signal_data.header.full_packet_size);
                        ESP_LOGI(TAG, "> signal_data data[0] %d ", signal_data.data[0]);
                        ESP_LOGI(TAG, "> signal_data data[1] %d ", signal_data.data[1]);
                        ESP_LOGI(TAG, "> Send %d  bytes ", signal_data.header.full_packet_size);
                    #endif 

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

}

