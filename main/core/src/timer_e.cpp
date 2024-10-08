#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "variables.h"

static const char *TAG = "esp32";

// Дескриптор таймера
esp_timer_handle_t periodic_timer = NULL;

esp_err_t stop_timer();

// Callback функция для таймера
void timer_callback(void* arg) {

    //ESP_LOGI(TAG, "__________________________________________timer_callback= %" PRId16, count); 

    var.timer.ready = 1;
}
//===================================================

void init_timer() {

    // Настройка параметров таймера
    esp_timer_create_args_t timer_args = {
        .callback = &timer_callback,
        .arg = NULL,
        .name = "my_periodic_timer"
    };
    var.timer.in_work = 0;
    var.timer.ready = 0;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &periodic_timer));
}
//===============================================================

void start_timer_mks(uint64_t period_mks) {
    var.timer.in_work = 1;
    var.timer.ready = 0;
    // Запуск периодического таймера с интервалом xx мс
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, period_mks)); // В микросекундах
}
//===============================================================

// Функция для остановки таймера
esp_err_t stop_timer() {

    var.timer.in_work = 0;
    var.timer.ready = 0;

    if (periodic_timer != NULL) {
        esp_err_t err = esp_timer_stop(periodic_timer);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to stop timer: %s", esp_err_to_name(err));
            return err;
        }
        ESP_LOGI(TAG, "Timer stopped successfully.");
        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "Timer handle is NULL. Cannot stop timer.");
        return ESP_ERR_INVALID_STATE;
    }
}
//===============================================================

// Функция для остановки таймера
esp_err_t restart_timer(uint64_t timeout_us) {

    var.timer.in_work = 1;
    var.timer.ready = 0;

    if (periodic_timer != NULL) {
        esp_err_t err = esp_timer_restart(periodic_timer, timeout_us);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to stop timer: %s", esp_err_to_name(err));
            return err;
        }
        ESP_LOGI(TAG, "Timer stopped successfully.");
        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "Timer handle is NULL. Cannot stop timer.");
        return ESP_ERR_INVALID_STATE;
    }
}
//===============================================================

esp_err_t start_once_timer(uint64_t timeout_us) {

    var.timer.in_work = 1;
    var.timer.ready = 0;

    if (periodic_timer != NULL) {
        esp_err_t err = esp_timer_start_once(periodic_timer, timeout_us);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to stop timer: %s", esp_err_to_name(err));
            return err;
        }
        ESP_LOGI(TAG, "Timer stopped successfully.");
        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "Timer handle is NULL. Cannot stop timer.");
        return ESP_ERR_INVALID_STATE;
    }   
}
//===============================================================

