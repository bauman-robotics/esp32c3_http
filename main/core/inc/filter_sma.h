#pragma once

#include "main.h"
#include <inttypes.h> 

/* Choose filter order */
/**
	* @brief Simple Moving Average (SMA) filter.
	* @note Before use define filter order.
	* @param[in] Input raw (unfiltered) value.
	* @retval Return filtered data.
	*/
#define MAX_FILTER_SMA_ORDER_I_MON 1000 

uint16_t Filter_SMA_V(uint16_t for_Filtered);	
uint16_t Filter_SMA_I(uint16_t for_Filtered);	



