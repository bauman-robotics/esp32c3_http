#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <math.h>
#include <inttypes.h> // Для использования макросов формата inttypes
#include <stdio.h>
#include "variables.h"
#include "main.h"
#include "ina226.h"
//=================================

// Очередь для передачи данных
static SignalData signal_data = {0}; // Инициализировать данные сигнала

//================================
extern const char *TAG;

int16_t calc_sine_post_data();
int16_t calc_sine_uart_data();
int16_t calc_sine_socket_data();
int16_t calc_sawtooth_socket_data();
int16_t calc_triangle_socket_data();

void signal_gen_task(void *pvParameters);
void Generate_Signal(SignalData *signal_data);
void Set_Header(SignalData* signalData); 
//===============================================================

int16_t calc_sine_post_data()
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * 2 * M_PI;
    int16_t sine_value = (int16_t)((sin(t) * (AMPLITUDE / 2)) + (AMPLITUDE / 2));

    s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

    return sine_value;
}
//===============================================================

int16_t calc_sine_uart_data()
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * 2 * M_PI;
    int16_t sine_value = (int16_t)((sin(t) * (AMPLITUDE / 2)) + (AMPLITUDE / 2));

    s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

    return sine_value;
}
//===============================================================
int16_t calc_sine_socket_data()
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * 2 * M_PI;
    int16_t sine_value = (int16_t)((sin(t) * (AMPLITUDE / 2)) + (AMPLITUDE / 2));

    s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

     return sine_value;
}


//===============================================================

int16_t calc_sawtooth_socket_data()
{
    //static uint32_t s_time = 0;
    static int16_t sawtooth_value = 0;

    // Calculate the current time position in the cycle
    // int16_t sawtooth_value = (int16_t)((float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * AMPLITUDE);
    // s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

    if (sawtooth_value < 1000) {
        sawtooth_value ++;
    } else {
        sawtooth_value = 0;
    }

    return sawtooth_value;
}
//===============================================================

int16_t calc_triangle_socket_data()
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS;
    int16_t triangle_value;

    if (t < 0.5) {
        triangle_value = (int16_t)(t * 2 * AMPLITUDE);
    } else {
        triangle_value = (int16_t)((1 - t) * 2 * AMPLITUDE);
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
        // for (int i = 0; i < var.count_vals_in_packet; i++) {
        //     //signal_data->data[i] = calc_triangle_socket_data();
        // }

        int index = 0; // Индекс для заполнения массива данных

        for (int i = 0; i < var.count_vals_in_packet; ) { // Используем i только для условия цикла
            Get_Voltage(); 
            if (var.ina226.voltage_is_valid) {
                signal_data->data[index] = var.ina226.voltage_i;
                index++; // Увеличиваем индекс только при валидном напряжении
            }
            
            // Убедитесь, что index не превышает размер массива
            if (index >= var.count_vals_in_packet) {
                break; // Выходим из цикла, если массив заполнен
            }

            // Задержка
            vTaskDelay(((float)var.signal_period / var.count_vals_in_packet) / portTICK_PERIOD_MS); 
        }        

    }
    // Установка заголовка
    Set_Header(signal_data);
    #ifdef DEBUG_LOG
    
    ESP_LOGI(TAG, "Generate_Signal_full_packet_size: %" PRId16, signal_data->header.full_packet_size);
    ESP_LOGI(TAG, "Generate_Signal_type: %" PRId8, signal_data->header.type);
    ESP_LOGI(TAG, "Generate_Signal_data[0]: %" PRId16, signal_data->data[0]);
    ESP_LOGI(TAG, "Generate_Signal_data[1]: %" PRId16, signal_data->data[1]);

    #endif 

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
//==============================================

// Функция для установки заголовка
void Set_Header(SignalData* signalData) {
    signalData->header.type = (uint16_t)BYNARY_PACKET_KEY; // Установка типа сообщения
    signalData->header.full_packet_size = var.count_vals_in_packet * sizeof(int16_t) + sizeof(PacketHeader); // Размер данных в байтах
}
//==============================================
