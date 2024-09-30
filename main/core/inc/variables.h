#pragma once

#include <inttypes.h> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "defines.h"
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
    char buf[NUM_ELEMENT_IN_PACKET*10+ 1]; // " data 1000"
} packet_type;
//======================================================================

typedef struct {   
    Led_Type leds;
    int signal_period;  // send period 
    packet_type packet;

} variables;

extern variables var;

//======================================================================

// Очередь для передачи данных
extern QueueHandle_t xQueue;