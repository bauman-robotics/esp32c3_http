#pragma once

#include <inttypes.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "defines.h"




//======================================================================

// Определяем структуру заголовка
typedef struct {
    uint16_t type;       // Тип сообщения      
    uint16_t full_packet_size;      // Размер данных в байтах
} PacketHeader;
//======================================================================
typedef struct {
    #ifndef DATA_TYPE_FLOAT
    PacketHeader header_int; // Заголовок
    int16_t data[MAX_NUM_ELEMENT_IN_PACKET]; // Массив для хранения данных сигнала
    #else 
    PacketHeader header_float; // Заголовок
    float   data_f[MAX_NUM_ELEMENT_IN_PACKET]; // Массив для хранения данных сигнала
    #endif 

    uint8_t ready;    // Флаг готовности данных 

 } SignalData;
//======================================================================
typedef struct {
    bool saw;
    bool sin;
    bool ina226;
    uint8_t flags;
} Led_Type;
//======================================================================

typedef struct {
    int count_el;
    char buf[SOCKET_BUF_TX_SIZE]; // " data 1000"
    bool type_hex;
} packet_type;
//======================================================================
typedef struct {   

   // input 
    uint16_t  I_lim_mA;
    float R_shunt_Om;
    //=== output 
    float     LSB_mA;
    uint16_t  CALIBR_VAL;
} calibr_t;
//======================================================================
typedef struct {   
    int16_t voltage_i;
    int16_t current_i;  
    int16_t power_i;    // for debug   
    float   voltage_f;   
    float   current_f;  // for debug
    float   power_f;    // for debug     
    bool is_init;
    bool value_is_valid;
    int64_t get_voltage_period_mks; 
    bool get_current;
    bool get_voltage;    
    bool get_power;        
    calibr_t calibr;
    TaskHandle_t task_handle;

} ina226_t;

//======================================================================
typedef struct {
    bool ready;
    bool in_work;
} Timer_t;

//======================================================================
typedef struct {
    bool wifi_state;
    bool serial_state;
    bool post_request_state;    
} need_state_t;

//======================================================================
typedef struct {
    bool wifi;
    bool serial;
    bool post_request;    
} state_t;

//======================================================================
typedef struct {
    int order_V;
    int order_I;
    int order_P;    
    bool enabled;
} filter_t;
//======================================================================
typedef struct {
    TaskHandle_t post_request_task;
    TaskHandle_t socket_task;
    TaskHandle_t serial_jtag_task;
} Task_Handles_t;
//======================================================================

//======================================================================

typedef struct {   
    Led_Type mode;
    int signal_period;  // send period 
    int count_vals_in_packet;      
    packet_type packet;
    ina226_t ina226;
    Timer_t timer;
    //bool data_type_float; 
    filter_t filter;
    Task_Handles_t handle;
    bool wifi_is_init;
    need_state_t need;
    state_t state;

    float last_val_for_post;
} variables;

extern variables var;

//======================================================================

// Очередь для передачи данных

extern QueueHandle_t xQueueSignalData;
extern QueueHandle_t xQueueSignalReady;


//======================================================================
