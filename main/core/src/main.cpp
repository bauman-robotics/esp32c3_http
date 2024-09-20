#include "nvs_flash.h"
#include "esp_log.h"
//============
#include "main.h"
#include "defines.h"
#include "http_func.h"
#include "led_blink.h"
//============
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//========================================
const char *TAG = "esp32";

//=====================================================================================
int alarm_interval   = 200; // 200 - 2 min, 00 sec
int cold = 1;
int hot = 1;
//=====================================================================================

// Объявление функций
void send_post_request(void *param);
void blink_led(void *param);
//=====================================================================================

// Задача для мигания светодиодом
void blink_led_task(void *pvParameters) {
    while (1) {
        blink_led();
        vTaskDelay(pdMS_TO_TICKS(10)); // Задержка на 10 миллисекунд
    }
}
//=====================================================================================

void send_post_request_task(void *pvParameters) {
    //vTaskDelay(pdMS_TO_TICKS(POST_REQUEST_PERIOD_MS)); // Задержка на xx секунд
    while (1) {
        static int sin_data = calc_sine_data(DNOT_USE_UART);
        send_post_request(cold, sin_data, alarm_interval); // Пример параметров, замените на реальные
        vTaskDelay(pdMS_TO_TICKS(POST_REQUEST_PERIOD_MS)); // Задержка на 1 секунду
    }
}
//=====================================================================================

extern "C" void app_main(void) {

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    nvs_flash_init();

    // Установите уровень логирования для Wi-Fi на VERBOSE
    esp_log_level_set("wifi", ESP_LOG_VERBOSE);

    wifi_init_sta();
    
    configure_led();

    // Создание задач
    xTaskCreate(send_post_request_task, "SendPostRequestTask", 4096, NULL, 5, NULL);
    xTaskCreate(blink_led_task, "BlinkLedTask", 2048, NULL, 5, NULL);   

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Добавьте задержку, чтобы избежать тайм-аута задачи
    }    
}


