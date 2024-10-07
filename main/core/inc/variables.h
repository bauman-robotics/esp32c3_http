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
    int16_t voltage_i;
    float   voltage_f;    
    bool is_init;
    bool voltage_is_valid;
    int64_t get_voltage_period_mks; 
} ina226_t;

//======================================================================
typedef struct {
    bool ready;
    bool in_work;
} Timer_t;

typedef struct {   
    Led_Type leds;
    int signal_period;  // send period 
    int count_vals_in_packet;      
    packet_type packet;
    ina226_t ina226;
    Timer_t timer;
    //bool data_type_float; 
} variables;

extern variables var;

//======================================================================

// Очередь для передачи данных

extern QueueHandle_t xQueueSignalData;
extern QueueHandle_t xQueueSignalReady;


//======================================================================
