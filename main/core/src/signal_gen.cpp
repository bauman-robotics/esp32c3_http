#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <math.h>
#include <inttypes.h> // Для использования макросов формата inttypes
#include <stdio.h>
#include "variables.h"
#include "main.h"
//=================================

// Очередь для передачи данных
static SignalData signal_data = {0}; // Инициализировать данные сигнала

//================================
extern const char *TAG;

int32_t calc_sine_post_data();
int32_t calc_sine_uart_data();
int calc_sine_socket_data();
int calc_sawtooth_socket_data();
int calc_triangle_socket_data();

void signal_gen_task(void *pvParameters);
void Generate_Signal(SignalData *signal_data);
//===============================================================

int32_t calc_sine_post_data()
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * 2 * M_PI;
    int32_t sine_value = (int32_t)((sin(t) * (AMPLITUDE / 2)) + (AMPLITUDE / 2));

    s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

    return sine_value;
}
//===============================================================

int32_t calc_sine_uart_data()
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * 2 * M_PI;
    int32_t sine_value = (int32_t)((sin(t) * (AMPLITUDE / 2)) + (AMPLITUDE / 2));

    s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

    return sine_value;
}
//===============================================================
int calc_sine_socket_data()
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * 2 * M_PI;
    int sine_value = (int)((sin(t) * (AMPLITUDE / 2)) + (AMPLITUDE / 2));

    s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

     return sine_value;
}


//===============================================================

int calc_sawtooth_socket_data()
{
    //static uint32_t s_time = 0;
    static int sawtooth_value = 0;

    // Calculate the current time position in the cycle
    // int sawtooth_value = (int)((float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * AMPLITUDE);
    // s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

    if (sawtooth_value < 1000) {
        sawtooth_value ++;
    } else {
        sawtooth_value = 0;
    }

    return sawtooth_value;
}
//===============================================================

int calc_triangle_socket_data()
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS;
    int triangle_value;

    if (t < 0.5) {
        triangle_value = (int)(t * 2 * AMPLITUDE);
    } else {
        triangle_value = (int)((1 - t) * 2 * AMPLITUDE);
    }

    s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

    return triangle_value;
}
//===============================================================

// Функция для генерации сигнала
void Generate_Signal(SignalData *signal_data) {
    if (var.leds.green) {
        for (int i = 0; i < var.count_vals_in_packet; i++) {
            signal_data->data[i] = calc_sine_socket_data();
        }
    } else if (var.leds.red) {
        for (int i = 0; i < var.count_vals_in_packet; i++) {
            signal_data->data[i] = calc_sawtooth_socket_data();
        }
    } else if (var.leds.blue) {
        for (int i = 0; i < var.count_vals_in_packet; i++) {
            signal_data->data[i] = calc_triangle_socket_data();
        }
    }
    signal_data->ready = 1; // Установить флаг готовности данных
}
//==============================================================================================================

void signal_gen_task(void *pvParameters) {

     while (1) {

        //ESP_LOGI(TAG, "Socket created, connecting to %s:%d", SOCKET_IP, SOCKET_PORT);
        //ESP_LOGI(TAG, "Successfully connected");

        if (var.leds.flags == LEDS_CONNECT_TO_SERVER_STATE) {

           

            Generate_Signal(&signal_data);
            
            // Отправка данных сигнала в очередь
            if (xQueueSend(xQueueSignalData, &signal_data, portMAX_DELAY) == pdPASS) {
                //ESP_LOGI(TAG, "Signal data sent to Queue successfully!");
            } else {
                ESP_LOGI(TAG, "Failed to send signal data!");
            }

            // Уведомление о готовности данных
            if (xQueueSend(xQueueSignalReady, &signal_data.ready, portMAX_DELAY) == pdPASS) {
                //ESP_LOGI(TAG, "Ready flag sent to Queue successfully!");
            } else {
                ESP_LOGI(TAG, "Failed to send ready flag!");
            }
            //ESP_LOGI(TAG, "Signal data[0]=%d", signal_data.data[0]);
            //ESP_LOGI(TAG, "Signal data[1]=%d", signal_data.data[1]);     
        }
        vTaskDelay(var.signal_period / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой

        // Пауза в 1 секунде между отправками
        //vTaskDelay(pdMS_TO_TICKS(1000));

    }
}