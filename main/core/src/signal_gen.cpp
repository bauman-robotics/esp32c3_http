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

#include "esp_timer.h" // Для высокоточных таймеров
#include "rom/ets_sys.h"  // Для esp_rom_delay_us
#include "timer_e.h"

//=================================

// Очередь для передачи данных
static SignalData signal_data = {0}; // Инициализировать данные сигнала

//================================
extern const char *TAG;

int16_t calc_sine_post_data();
int16_t calc_sine_socket_data();
float calc_sine_socket_data_float();
int16_t calc_sawtooth_socket_data();
float calc_sawtooth_socket_data_float();
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

float calc_sine_socket_data_float()
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * 2 * M_PI;
    float sine_value = ((sin(t) * (AMPLITUDE / 2)) + (AMPLITUDE / 2));

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

float calc_sawtooth_socket_data_float()
{
    static float sawtooth_value = 0;

    if (sawtooth_value < 10) {
        sawtooth_value += 0.01;
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

    static uint64_t timer_period_mks = (uint64_t)var.ina226.get_voltage_period_mks;
    static uint64_t old_timer_period_mks = timer_period_mks;    
    

    if (var.mode.sin) {
        for (int i = 0; i < var.count_vals_in_packet; i++) {

            #ifndef DATA_TYPE_FLOAT     
                signal_data->data[i] = calc_sine_socket_data();
            #else
                signal_data->data_f[i] = calc_sine_socket_data_float();          
            #endif
        }
    } else if (var.mode.saw) {
        for (int i = 0; i < var.count_vals_in_packet; i++) {

            #ifndef DATA_TYPE_FLOAT     
                signal_data->data[i] = calc_sawtooth_socket_data();
            #else
                signal_data->data_f[i] = calc_sawtooth_socket_data_float();          
            #endif            
        }
    } else if (var.mode.ina226) {  // Voltage, Current, Power 

        if (var.ina226.is_init) {

            int index = 0; // Индекс для заполнения массива данных

            for (int i = 0; i < var.count_vals_in_packet; ) { // Используем i только для условия цикла

                //==========================================================
                // Задержка
                var.ina226.get_voltage_period_mks = (uint64_t)(1000 * (float)var.signal_period / var.count_vals_in_packet);

                timer_period_mks = var.ina226.get_voltage_period_mks;
                
                if (timer_period_mks != old_timer_period_mks) {
                    old_timer_period_mks = timer_period_mks;

                    if (var.timer.in_work) {
                        restart_timer(timer_period_mks);
                        ESP_LOGI(TAG, "_______________start_timer_mks : %" PRId64, timer_period_mks);
                    }
                }

                if (!var.timer.in_work) {
                    start_timer_mks(timer_period_mks); 
                    ESP_LOGI(TAG, "_________________start_timer_mks : %" PRId64, timer_period_mks);
                }

                while (!var.timer.ready) {

                }
                var.timer.ready = 0;
                //==========================================================
                float value = 0;
                if (var.ina226.get_voltage) {
                    value = Get_Voltage(); 
                } else if  (var.ina226.get_current) {
                    value = Get_Current();
                } else if  (var.ina226.get_power) {
                    value = Get_Power();
                }
                
                if (var.ina226.value_is_valid) {

                    #ifndef DATA_TYPE_FLOAT     
                        signal_data->data[index] = var.ina226.voltage_i;
                    #else
                        signal_data->data_f[index] = value;          
                    #endif       
                    //
                    index++; // Увеличиваем индекс только при валидном напряжении                        
                }

                var.last_val_for_post = value;
                // Убедитесь, что index не превышает размер массива
                if (index >= var.count_vals_in_packet) {
                    break; // Выходим из цикла, если массив заполнен
                }
            }  
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

    //ESP_LOGI(TAG, "signal_gen_task  create()");
    while (1) {

        if (var.mode.flags == LEDS_CONNECT_TO_SERVER_STATE) {

            Generate_Signal(&signal_data);
            //ESP_LOGI(TAG, "Generate_Signal");
            // Отправка данных сигнала в очередь
            if (xQueueSend(xQueueSignalData, &signal_data, portMAX_DELAY)) {
                //ESP_LOGI(TAG, "Signal data sent to Queue successfully!");
            } else {
                ESP_LOGI(TAG, "Failed to send signal data!");
            }

            // Уведомление о готовности данных
            if (xQueueSend(xQueueSignalReady, &signal_data.ready, portMAX_DELAY)) {
                //ESP_LOGI(TAG, "Ready flag sent to Queue successfully!");
            } else {
                ESP_LOGI(TAG, "Failed to send ready flag!");
            }
            //ESP_LOGI(TAG, "Signal data[0]=%d", signal_data.data[0]);
            //ESP_LOGI(TAG, "Signal data[1]=%d", signal_data.data[1]);     
        }

        int delay_ms = var.signal_period;
        if (!var.mode.ina226) {            
            delay_ms = var.signal_period;        
        } else {
            delay_ms = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(delay_ms));

        //vTaskDelay(var.signal_period / portTICK_PERIOD_MS);  // Подождать перед повторной попыткой
        //vTaskDelay(pdMS_TO_TICKS(var.signal_period));  // Подождать перед повторной попыткой
    }
}
//==============================================

// Функция для установки заголовка
void Set_Header(SignalData* signalData) {
    #ifndef DATA_TYPE_FLOAT 
        signalData->header_int.type = (uint16_t)BINARY_PACKET_INT_KEY; // Установка типа сообщения
        signalData->header_int.full_packet_size = var.count_vals_in_packet * sizeof(int16_t) + sizeof(PacketHeader); // Размер данных в байтах
    #else
        signalData->header_float.type = (uint16_t)BINARY_PACKET_FLOAT_KEY; // Установка типа сообщения
        signalData->header_float.full_packet_size = var.count_vals_in_packet * sizeof(float) + sizeof(PacketHeader); // Размер данных в байтах        
    #endif
}
//==============================================
