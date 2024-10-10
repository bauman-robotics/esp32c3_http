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

#include "ina226.h"
#include "i2c.h"
#include "driver/i2c.h"
#include "timer_e.h"

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

void ina226_init_task(void *pvParameters) {
    while (1) {
        // ESP_LOGI(TAG, "var.ina226.is_init = %" PRId16, (int)var.ina226.is_init);
        if (!var.ina226.is_init) {

            ina226_Calc_Coeff(); 
            ina226_Calibr_Logs();
            var.ina226.is_init = ina226_init(I2C_CONTROLLER_0);
            ESP_LOGI(TAG, "var.ina226.is_init = %" PRId16, (int)var.ina226.is_init);            
        } else {
            // Завершение задачи
            var.ina226.task_handle = 0;
            ESP_LOGI(TAG, " ina226 vTaskDelete ");            
            vTaskDelete(NULL);

        }

        vTaskDelay(pdMS_TO_TICKS(INA226_TRY_INIT_PERIOD_MS)); // Задержка на xx секунд
    }
}
//=====================================================================================

void log_task(void *pvParameters) {

    while (1) {
        
	    //ESP_LOGI(TAG, "var.ina226.calibr.CALIBR_VAL= %d", (uint16_t)var.ina226.calibr.CALIBR_VAL);
        // ESP_LOGI(TAG, "var.ina226.LSB_mkA = %.001f", var.ina226.calibr.LSB_mA * 1000);   
        //ESP_LOGI(TAG, "var.ina226.current_i = %d", var.ina226.current_i);  
        //ESP_LOGI(TAG, "current_mA = %.4f", var.ina226.current_f);
        // //ESP_LOGI(TAG, "power_i = %d", var.ina226.power_i);          

            
        // ina226_Calibr_Logs();

        //ESP_LOGI(TAG, "CURRENT_COEF * 1000= %"                PRId16, (int)(INA226_CURRENT_COEFF * 1000));
        //ESP_LOGI(TAG, "var.count_vals_in_packet= %"         PRId16, var.count_vals_in_packet);
        //ESP_LOGI(TAG, "var.ina226.get_voltage_period_mks= %" PRId16, var.ina226.get_voltage_period_mks);        
        vTaskDelay(pdMS_TO_TICKS(LOG_TASK_PERIOD_MS)); // Задержка на xx секунд      
    }
}
//=====================================================================================

extern "C" void app_main(void) {


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
    #ifdef GET_CURRENT_DEFAULT
        var.ina226.get_current = 1;
    #else 
        var.ina226.get_current = 0;
    #endif 

    init_timer(); 

    #ifdef INA226_ENABLE
        var.ina226.task_handle = NULL;    
        ina226_Set_Coeff_Default();
        
        i2c_init(I2C_CONTROLLER_0, I2C_SDA_PIN, I2C_SCL_PIN);
        var.ina226.is_init = ina226_init(I2C_CONTROLLER_0);
        ESP_LOGI(TAG, "var.ina226.is_init = %d", (int)var.ina226.is_init);

        // ina226_Set_Coeff_Default();
        // ina226_Calc_Coeff(); 
        // ina226_Calibr_Logs();
        
    #endif 

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
        xTaskCreate(socket_task, "socket_task", 4096 + 2048, NULL, 5, NULL);
    #endif 
    //=========================================

    #ifdef SEND_UART_SIN_ENABLE  
        xTaskCreate(uart_sin_send_task, "Uart_task", 2048, NULL, 5, NULL);   
    #endif 

    #ifdef INA226_ENABLE    
        xTaskCreate(ina226_init_task, "ina226_init_task", 2048, NULL, 5,  &var.ina226.task_handle);
    #endif 

    #ifdef LOG_TASK_ENABLE    
        xTaskCreate(log_task, "log_task", 2048, NULL, 5, NULL);
    #endif 

    var.filter.order_V = FILTER_ORDER_V;
    var.filter.order_I = FILTER_ORDER_I;
    var.filter.order_P = FILTER_ORDER_P;    
    #ifdef FILTER_V_I_P_ENABLE
        var.filter.enabled = 1;
    #else 
        var.filter.enabled = 0;
    #endif 

    //=========================================
    xQueueSignalData    = xQueueCreate(10, sizeof(SignalData));
    xQueueSignalReady   = xQueueCreate(10, sizeof(uint8_t));
  
    if (xQueueSignalData  == NULL || xQueueSignalReady == NULL ) {
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

