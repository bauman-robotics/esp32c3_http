#pragma once

//========================== Анализ стека функции ==========================
//=== UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(NULL);  ===

// Эта функция возвращает минимальное количество свободных слов стека, которые были доступны для задачи с момента ее создания. 
// Это помогает понять, сколько стека используется в пиковых ситуациях.

// void send_post_request_task(void *pvParameters) {
//     while (1) {
//         send_post_request(25, 30, 60); // Пример параметров, замените на реальные
//         UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(NULL);
//         ESP_LOGI(TAG, "send_post_request_task: highWaterMark = %u", highWaterMark);
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Задержка на 1 секунду
//     }
// }

// Если значение highWaterMark слишком маленькое (например, менее 100 слов), увеличьте размер стека.
// Если значение highWaterMark остается большим (например, более 500 слов), можно уменьшить размер стека для оптимизации использования памяти.

//========================================================================