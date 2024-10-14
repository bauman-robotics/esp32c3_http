#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "variables.h"
#include "esp_log.h"

#include <netdb.h>  // isdigit()
#include "variables.h"


static const char *TAG = "ex";

extern void ina226_init_task(void *pvParameters);

void Pars_Cmd(char * rx_buf);
bool process_keyword(char *rx_buf, const char *keyword, int *value);

//==============================================================================================================

//Функция анализа строки и установки флагов
void Pars_Cmd(char *rx_buf) {

    // Проверка на нулевой указатель
    if (rx_buf == NULL) {
        ESP_LOGI(TAG, "rx_buf is NULL");
        return;
    }    
    //ESP_LOGI(TAG, "________________________Pars_Cmd()");
 

    // Определение вариантов соответствия
    const char *options[] = {"Saw", "Sin", "Voltage", "Current", "Power", "HEX", "ASCII"};
    int count = sizeof(options) / sizeof(options[0]);
    // Анализ строки и установка флагов

    for (int i = 0; i < count; i++) {
        //if (strstr(rx_buf, options[i]) != NULL) {

        //ESP_LOGI(TAG, "___Rx=%s    Var=%s", rx_buf, options[i]);

        if (strcasestr(rx_buf, options[i]) != NULL) {          
            switch (i) {
                case 0:
                    ESP_LOGI(TAG, "________________________Saw");

                    var.mode.saw   = 1;
                    var.mode.sin   = 0;
                    var.mode.ina226  = 0;
                    break;
                //=====================    
                case 1:
                    ESP_LOGI(TAG, "________________________Sin");

                    var.mode.saw   = 0;
                    var.mode.sin   = 1;
                    var.mode.ina226  = 0;
                    break;
                //===================== 
                case 2:
                    ESP_LOGI(TAG, "________________________Voltage");

                    var.mode.saw  = 0;
                    var.mode.sin  = 0;
                    var.mode.ina226  = 1;   
                    var.ina226.get_voltage = 1;
                    var.ina226.get_current = 0;
                    var.ina226.get_power   = 0;   
                    break;
                //===================== 
                case 3:
                    ESP_LOGI(TAG, "________________________Current");

                    var.mode.saw   = 0;
                    var.mode.sin   = 0;
                    var.mode.ina226  = 1;   

                    var.ina226.get_voltage = 0;
                    var.ina226.get_current = 1;
                    var.ina226.get_power   = 0;   
    
                    break;
                //===================== 
                case 4:
                    ESP_LOGI(TAG, "________________________Power");

                    var.mode.saw   = 0;
                    var.mode.sin   = 0;
                    var.mode.ina226  = 1;   

                    var.ina226.get_voltage = 0;
                    var.ina226.get_current = 0;
                    var.ina226.get_power   = 1;   
    
                    break;                    
                //=====================                 
                case 5:
                    ESP_LOGI(TAG, "________________________HEX");

                    var.packet.type_hex = 1;
                 
                    break;
                //===================== 
                case 6:
                    ESP_LOGI(TAG, "________________________ASCII");

                    var.packet.type_hex = 0;

                    break;  
                   // for de        
                // default:
                //     ESP_LOGI(TAG, "__------------ Not Responced:%s", rx_buf);

                //     break;                                                      
            }
        }
    }

    // Обработка ключевого слова "PER"
    process_keyword(rx_buf, "PER", &var.signal_period);

    // Обработка ключевого слова "NUM"
    process_keyword(rx_buf, "NUM", &var.count_vals_in_packet);

    // Обработка ключевого слова "I_FILTER_ORDER"
    if (process_keyword(rx_buf, "I_FILTER_ORDER", &var.filter.order_I)) {
        var.filter.order_P = var.filter.order_I;
    }

    // Обработка ключевого слова "V_FILTER_ORDER"
    process_keyword(rx_buf, "V_FILTER_ORDER", &var.filter.order_V);

    // Обработка ключевого слова "I_LIM_SET"
    if (process_keyword(rx_buf, "I_LIM_SET", (int*)&var.ina226.calibr.I_lim_mA)) {
        var.ina226.is_init = 0;
        ESP_LOGI(TAG, "var.ina226.is_init = %d", (int)var.ina226.is_init);
        ESP_LOGI(TAG, "var.ina226.task_handle = %d", (int)var.ina226.task_handle);
        if (var.ina226.task_handle == NULL) {
            // Создаем задачу
            xTaskCreate(ina226_init_task, "ina226_init_task", 2048, NULL, 5, &var.ina226.task_handle);

            // ina226_Calc_Coeff(); 
            // ina226_Calibr_Logs();
            // var.ina226.is_init = ina226_init(I2C_CONTROLLER_0);
            // ESP_LOGI(TAG, "var.ina226.is_init = %" PRId16, (int)var.ina226.is_init);
        }
    }   

}
//==============================================================================================================

bool process_keyword(char *rx_buf, const char *keyword, int *value) {
    bool result = 0;
    char *keyword_ptr = strstr(rx_buf, keyword);
    if (keyword_ptr != NULL) {
        keyword_ptr += strlen(keyword); // Переходим за ключевое слово

        // Пропускаем пробелы и недопустимые символы
        while (*keyword_ptr != '\0' && !isdigit((unsigned char)*keyword_ptr)) {
            keyword_ptr++;
        }

        if (*keyword_ptr != '\0') {
            // Извлекаем целое значение после ключевого слова
            *value = atoi(keyword_ptr);
            ESP_LOGI(TAG, "________________________%s value: %d", keyword, *value);
            result = 1;
        }
    }
    return result;
}
//==============================================================================================================

