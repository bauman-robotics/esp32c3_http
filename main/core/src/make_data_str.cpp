#include <stdio.h>
#include <string.h>

//================================================================================================================

int Add_Number_To_String(char *str, int number, const char *prefix, int *element_count, int max_elements);
int Add_Number_To_String_Float(char *str, float number_f, const char *prefix, int *element_count, int max_elements);

//================================================================================================================

// Функция для добавления числа в строку и проверки заполненности
int Add_Number_To_String(char *str, int number, const char *prefix, int *element_count, int max_elements) {
    char number_str[12]; // Буфер для хранения строкового представления числа
    sprintf(number_str, "%d%s", number, " "); // Конвертация числа в строку

    // Добавляем префикс и число
    strcat(str, prefix);
    strcat(str, number_str);

    // Увеличиваем счетчик количества элементов
    (*element_count)++;

    //ESP_LOGI(TAG, "Add data to string %d", number);

    // Проверяем, достигли ли мы максимального количества элементов
    if (*element_count >= max_elements) {
        //ESP_LOGI(TAG, "Packet full");
        return 1; // Строка заполнена        
    }

    return 0; // Строка еще не заполнена
}
//=================================================================================

// Функция для добавления числа в строку и проверки заполненности
int Add_Number_To_String_Float(char *str, float number_f, const char *prefix, int *element_count, int max_elements) {
    char number_str[12]; // Буфер для хранения строкового представления числа
    sprintf(number_str, "%.2f%s", number_f, " "); // Конвертация числа в строку

    // Добавляем префикс и число
    strcat(str, prefix);
    strcat(str, number_str);

    // Увеличиваем счетчик количества элементов
    (*element_count)++;

    //ESP_LOGI(TAG, "Add data to string %d", number);

    // Проверяем, достигли ли мы максимального количества элементов
    if (*element_count >= max_elements) {
        //ESP_LOGI(TAG, "Packet full");
        return 1; // Строка заполнена        
    }

    return 0; // Строка еще не заполнена
}
//=================================================================================

