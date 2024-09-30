#pragma once


void configure_led(void);
void blink_led(void);

int32_t calc_sine_uart_data();
int32_t calc_sine_post_data();
int32_t calc_sine_socket_data();
int32_t calc_sawtooth_socket_data();
int32_t calc_triangle_socket_data();

