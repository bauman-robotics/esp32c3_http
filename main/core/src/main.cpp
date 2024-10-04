#include "nvs_flash.h"
#include "esp_log.h"
//============
#include "main.h"
#include "defines.h"
#include "http_func.h"
#include "led_blink.h"
#include "web_socket.h"
#include "signal_gen.h"
#include "variables.h"
//============
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


//========================================
const char *TAG = "esp32";

static int16_t post_sin_data;
static int16_t uart_sin_data;
//=====================================================================================
int16_t alarm_interval   = 200; // 200 - 2 min, 00 sec
int16_t cold = 1;
int16_t hot = 1;
//=====================================================================================

// Объявление функций
//void send_post_request(void *param);
void blink_led(void *param);
//=====================================================================================

// Задача для мигания светодиодом
void blink_led_task(void *pvParameters) {
    while (1) {
        #ifdef ESP32_C3
            blink_led_C3();
        #else 
            blink_led();
        #endif    

        vTaskDelay(pdMS_TO_TICKS(10)); // Задержка на 10 миллисекунд
    }
}
//=====================================================================================

void send_post_request_task(void *pvParameters) {
    //vTaskDelay(pdMS_TO_TICKS(POST_REQUEST_PERIOD_MS)); // Задержка на xx секунд
    while (1) {
        post_sin_data = calc_sine_post_data();

        //ESP_LOGI(TAG, "data %" PRId32, post_sin_data);

        send_post_request(cold, post_sin_data, alarm_interval); // Пример параметров, замените на реальные
         
        vTaskDelay(pdMS_TO_TICKS(POST_REQUEST_PERIOD_MS)); // Задержка на 1 секунду
    }
}
//=====================================================================================

void uart_sin_send_task(void *pvParameters) {
    while (1) {
        uart_sin_data = calc_sine_uart_data();
        ESP_LOGI(TAG, "data %" PRId16, uart_sin_data);
        vTaskDelay(pdMS_TO_TICKS(UART_SIN_SEND_PERIOD_MS)); // Задержка на 1 секунду
    }
}
//=====================================================================================

// Ваша собственная функция вывода логов
// int my_log_vprintf(const char *fmt, va_list args) {
//     return vprintf(fmt, args);
// }
//=====================================================================================

extern "C" void app_main(void) {

    // // Установите свою функцию временных меток
    // esp_log_set_vprintf(my_log_vprintf);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    nvs_flash_init();

    // Установите уровень логирования для Wi-Fi на VERBOSE
    esp_log_level_set("wifi", ESP_LOG_VERBOSE);

    #if defined(POST_REQUEST_ENABLE) | defined(SOCKET_CLIENT_ENABLE)
        wifi_init_sta();
    #endif 
    
    #ifdef ESP32_C3
        configure_led_C3();
    #else
        configure_led();
    #endif
    
    var.signal_period        = SOCKET_SEND_PERIOD_MS;
    var.count_vals_in_packet = NUM_ELEMENT_IN_PACKET;
    #ifdef BINARY_PACKET 
        var.packet.type_hex = 1;
    #else 
        var.packet.type_hex = 0;
    #endif 

    // Создание задач
    #ifdef POST_REQUEST_ENABLE
        xTaskCreate(send_post_request_task, "SendPostRequestTask", 4096, NULL, 5, NULL);
    #endif 
    //=========================================

    #ifdef LED_BLINK_ENABLE
        xTaskCreate(blink_led_task, "BlinkLedTask", 2048, NULL, 5, NULL);   
    #endif 
    //=========================================

    #ifdef SOCKET_CLIENT_ENABLE    
        xTaskCreate(socket_task, "socket_task", 4096, NULL, 5, NULL);
    #endif 
    //=========================================

    #ifdef SEND_UART_SIN_ENABLE  
        xTaskCreate(uart_sin_send_task, "Uart_task", 2048, NULL, 5, NULL);   
    #endif 

    //=========================================
    xQueueSignalData = xQueueCreate(10, sizeof(SignalData));
    xQueueSignalReady = xQueueCreate(10, sizeof(uint8_t));

    if (xQueueSignalData == NULL || xQueueSignalReady == NULL) {
        ESP_LOGE(TAG, "Failed to create queues");
        return;
    }
    //=========================================
    

    #ifdef SIGNAL_GEN_TASK_EN  
        xTaskCreate(signal_gen_task, "signal_gen_task", 2048, NULL, 5, NULL);   
    #endif 



    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Добавьте задержку, чтобы избежать тайм-аута задачи
    }    
}

