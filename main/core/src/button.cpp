#include "esp_log.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
//============
#include "variables.h"
#include "main.h"
#include "uart.h"
#include "web_socket.h"
#include "http_func.h"

static const char *TAG = "Button";

#define BUTTON_PIN GPIO_NUM_9  // Пин кнопки на ESP32-C6-Zero

static int button_press_count = 0;
static bool last_button_state = true;

void Start_Socket_Task();
void Start_Serial_Task();
void Start_Post_Request_Task(); 

void send_post_request_task(void *pvParameters);


void Button_Task(void *pvParameters)
{
    // Настройка GPIO для кнопки
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    while (1) {
        bool current_button_state = gpio_get_level(BUTTON_PIN);

        if (last_button_state == true && current_button_state == false) {
            // Кнопка была нажата
            
            //printf("Button pressed! Count: %lu\n", button_press_count);
            //ESP_LOGI(TAG, "Button pressed! Count: %d\n", button_press_count);

            switch (button_press_count) {

                case 0:
                    button_press_count = 1;
                    ESP_LOGI(TAG, "Start_Socket_Task \n");
                    Start_Socket_Task();
                    break;
                case 1:
                    button_press_count = 2;
                    ESP_LOGI(TAG, "post_request_state \n");
                    Start_Post_Request_Task();
                    //var.need.post_request_state = 1;              
                    break;    
                case 2:                                               
                    button_press_count = 0;
                    ESP_LOGI(TAG, "Start_Serial_Task \n");
                    Start_Serial_Task();
                    break;
                

            }
        }
        last_button_state = current_button_state;

        //=========================================
        if (var.need.serial_state) {
            var.need.serial_state = 0;
            Start_Serial_Task();
        } 
        //=========================================        
        if (var.need.wifi_state) {
            var.need.wifi_state = 0;
            Start_Socket_Task();
        }
        //=========================================        
        if (var.need.post_request_state) {
            var.need.post_request_state = 0;
            Start_Post_Request_Task();
        }        
        //=========================================

        vTaskDelay(pdMS_TO_TICKS(50)); // Небольшая задержка для дебаунсинга
    }
}
//============================================================================

void Start_Socket_Task() {

    ESP_LOGI(TAG, "Wifi mode"); 
    
    if (var.handle.serial_jtag_task) {
        vTaskDelete(var.handle.serial_jtag_task);
        var.handle.serial_jtag_task = 0;
    }

    if (var.handle.post_request_task) {
        vTaskDelete(var.handle.post_request_task);
        var.handle.post_request_task = 0;
    }
    if (!var.wifi_is_init) {

        wifi_init_sta();
    }

    if ((var.wifi_is_init) && (!var.handle.socket_task)) {
        xTaskCreate(socket_task, "socket_task", 4096 + 2048, NULL, 5, &var.handle.socket_task);

        var.state.serial       = 0;
        var.state.wifi         = 1;
        var.state.post_request = 0;        
    }   
}
//============================================================================

void Start_Serial_Task() {

    ESP_LOGI(TAG, "Serial mode");  


    if (var.handle.socket_task) {
        vTaskDelete(var.handle.socket_task);
        var.handle.socket_task = 0;
    }

    if (var.handle.post_request_task) {
        vTaskDelete(var.handle.post_request_task);
        var.handle.post_request_task = 0;
    }

    if (!var.handle.serial_jtag_task) {

        xTaskCreate(usb_serial_jtag_task, "USB SERIAL JTAG_task", 4096 + 2048, NULL, 5, &var.handle.serial_jtag_task);

        var.state.serial       = 1;
        var.state.wifi         = 0;
        var.state.post_request = 0;        
    }
    
}
//============================================================================

void Start_Post_Request_Task() {

    ESP_LOGI(TAG, "Post Request mode");  


    if (var.handle.socket_task) {
        vTaskDelete(var.handle.socket_task);
        var.handle.socket_task = 0;
    }

    if (var.handle.serial_jtag_task) {
        vTaskDelete(var.handle.serial_jtag_task);
        var.handle.serial_jtag_task = 0;
    }

    if (!var.handle.post_request_task) {

        xTaskCreate(send_post_request_task, "SendPostRequestTask", 4096, NULL, 5, &var.handle.post_request_task);

        var.state.serial       = 0;
        var.state.wifi         = 0;
        var.state.post_request = 1;
    }
    
}
//============================================================================


