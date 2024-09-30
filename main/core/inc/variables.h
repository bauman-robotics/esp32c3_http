#pragma once

#include <inttypes.h> 
//======================================================================

typedef struct {
    bool red;
    bool green;
    bool blue;
    uint8_t flags;

} Led_Type;
//======================================================================

typedef struct {   
    Led_Type leds;
    int per_value;  // send period 
} variables;

extern variables var;