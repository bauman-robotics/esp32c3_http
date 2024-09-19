#include "nvs_flash.h"
#include "esp_log.h"
//============
#include "defines.h"
#include "http_func.h"
#include "led_blink.h"
//========================================
const char *TAG = "esp32";

//=====================================================================================
int alarm_interval   = 200; // 200 - 2 min, 00 sec
int cold = -1;
int hot = -1;
//=====================================================================================

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    nvs_flash_init();
    wifi_init_sta();
    wait_for_ip();

    cold = 25; // Example value
    hot = 30;  // Example value

    send_post_request(cold, hot, alarm_interval);
    
    configure_led();

    while (1) {
        int data = send_sine_values();
        send_post_request(cold, data, alarm_interval);
        //blink_led();
        vTaskDelay(3000 / portTICK_PERIOD_MS); // Update every 10 milliseconds
    }    
}


