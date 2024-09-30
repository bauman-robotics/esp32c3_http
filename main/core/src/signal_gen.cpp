// #include "esp_log.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/uart.h"
#include <math.h>
#include <inttypes.h> // Для использования макросов формата inttypes
#include <stdio.h>
// #include "driver/gpio.h"
// #include "led_strip.h"
// #include "sdkconfig.h"

// #include "variables.h"
#include "main.h"
//=================================
extern const char *TAG;


int32_t calc_sine_post_data();
int32_t calc_sine_uart_data();
int32_t calc_sine_socket_data();
int32_t calc_sawtooth_socket_data();
int32_t calc_triangle_socket_data();


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
int32_t calc_sine_socket_data()
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * 2 * M_PI;
    int32_t sine_value = (int32_t)((sin(t) * (AMPLITUDE / 2)) + (AMPLITUDE / 2));

    s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

     return sine_value;
}


//===============================================================

int32_t calc_sawtooth_socket_data()
{
    static uint32_t s_time = 0;
    static int32_t sawtooth_value = 0;

    // Calculate the current time position in the cycle
    // int32_t sawtooth_value = (int32_t)((float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS * AMPLITUDE);
    // s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

    if (sawtooth_value < 1000) {
        sawtooth_value ++;
    } else {
        sawtooth_value = 0;
    }

    return sawtooth_value;
}
//===============================================================

int32_t calc_triangle_socket_data()
{
    static uint32_t s_time = 0;

    // Calculate the current time position in the cycle
    float t = (float)(s_time % SIN_PERIOD_MS) / SIN_PERIOD_MS;
    int32_t triangle_value;

    if (t < 0.5) {
        triangle_value = (int32_t)(t * 2 * AMPLITUDE);
    } else {
        triangle_value = (int32_t)((1 - t) * 2 * AMPLITUDE);
    }

    s_time += (SIN_PERIOD_MS / SIN_VALUES_COUNT); // Increment time

    return triangle_value;
}
//===============================================================




