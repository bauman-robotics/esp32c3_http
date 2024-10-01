#pragma once

#include <inttypes.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "defines.h"
//======================================================================
typedef struct {
    int16_t data[MAX_NUM_ELEMENT_IN_PACKET]; // Массив для хранения данных сигнала
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
} packet_type;
//======================================================================

typedef struct {   
    Led_Type leds;
    int signal_period;  // send period 
    int count_vals_in_packet;      
    packet_type packet;
} variables;

extern variables var;

//======================================================================

// Очередь для передачи данных

extern QueueHandle_t xQueueSignalData;
extern QueueHandle_t xQueueSignalReady;