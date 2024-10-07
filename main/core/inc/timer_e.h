#pragma once

void init_timer();
void start_timer_mks(uint64_t period_mks); 
esp_err_t start_once_timer(uint64_t timeout_us);
esp_err_t stop_timer();
esp_err_t restart_timer(uint64_t timeout_us); 
