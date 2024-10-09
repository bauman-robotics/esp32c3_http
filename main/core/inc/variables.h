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
    bool red;
    bool green;
    bool blue;
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
    int   I_lim_mA;
    float R_shunt_Om;
    //=== output 
    //float LSB_A;
    float LSB_mA;
    uint16_t   CALIBR_VAL;
    //float Current_coeff;
} calibr_t;
//======================================================================
typedef struct {   
    int16_t voltage_i;
    int16_t current_i;    
    float   voltage_f;    
    bool is_init;
    bool voltage_is_valid;
    int64_t get_voltage_period_mks; 
    bool get_current;
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
    int order_V;
    int order_I;
    bool enabled;
} filter_t;

//======================================================================

typedef struct {   
    Led_Type leds;
    int signal_period;  // send period 
    int count_vals_in_packet;      
    packet_type packet;
    ina226_t ina226;
    Timer_t timer;
    //bool data_type_float; 
    filter_t filter;
} variables;

extern variables var;

//======================================================================

// Очередь для передачи данных

extern QueueHandle_t xQueueSignalData;
extern QueueHandle_t xQueueSignalReady;


//======================================================================
