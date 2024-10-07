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
    PacketHeader header; // Заголовок
    int16_t data[MAX_NUM_ELEMENT_IN_PACKET]; // Массив для хранения данных сигнала
    uint8_t ready;    // Флаг готовности данных  
    uint8_t go; 
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
} variables;

extern variables var;

//======================================================================

// Очередь для передачи данных

extern QueueHandle_t xQueueSignalData;
extern QueueHandle_t xQueueSignalReady;


//======================================================================
