#pragma once

#include <inttypes.h> 

int16_t calc_sine_uart_data();
int16_t calc_sine_post_data();
int16_t calc_sine_socket_data();
int16_t calc_sawtooth_socket_data();
int16_t calc_triangle_socket_data();

void signal_gen_task(void *pvParameters);
