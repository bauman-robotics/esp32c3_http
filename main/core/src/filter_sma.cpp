#include "filter_sma.h"
#include "variables.h"


// uint16_t filterOrder_V = 100;
// uint16_t filterOrder_I = 100;
// uint16_t filterOrder_P = 100;

uint32_t Filter_Buffer_V[MAX_FILTER_SMA_ORDER_I_MON] = {0,};
uint32_t Filter_Buffer_I[MAX_FILTER_SMA_ORDER_I_MON] = {0,};
uint32_t Filter_Buffer_P[MAX_FILTER_SMA_ORDER_I_MON] = {0,};
/**

  * @brief Simple Moving Average (SMA) filter.
  * @note Before use define filter order.
  * @param[in] Input raw (unfiltered) value.
  * @retval Return filtered data.
  */

 #define filterOrder_V (var.filter.order_V)
 #define filterOrder_I (var.filter.order_I)
 #define filterOrder_P (var.filter.order_P)

uint16_t Filter_SMA_V(uint16_t for_Filtered)
{
	/* Load new value */
	Filter_Buffer_V[filterOrder_V - 1] = for_Filtered;
	/* For output value */
	uint32_t Output = 0;
	/* Sum */
	for(uint16_t i = 0; i < filterOrder_V; i++) {
		Output += Filter_Buffer_V[i];
	}
	/* Divide */
	Output /= filterOrder_V;
	/* Left Shift */
	for(uint16_t i = 0; i < filterOrder_V; i++) {
		Filter_Buffer_V[i] = Filter_Buffer_V[i+1];
	}
	/* Return filtered value */
	return (uint16_t) Output;
}
//====================================================================

uint16_t Filter_SMA_I(uint16_t for_Filtered)
{
	/* Load new value */
	Filter_Buffer_I[filterOrder_I - 1] = for_Filtered;
	/* For output value */
	uint32_t Output = 0;
	/* Sum */
	for(uint16_t i = 0; i < filterOrder_I; i++) {
		Output += Filter_Buffer_I[i];
	}
	/* Divide */
	Output /= filterOrder_I;
	/* Left Shift */
	for(uint16_t i = 0; i < filterOrder_I; i++) {
		Filter_Buffer_I[i] = Filter_Buffer_I[i+1];
	}
	/* Return filtered value */
	return (uint16_t) Output;
}
//====================================================================

uint16_t Filter_SMA_P(uint16_t for_Filtered)
{
	/* Load new value */
	Filter_Buffer_P[filterOrder_P - 1] = for_Filtered;
	/* For output value */
	uint32_t Output = 0;
	/* Sum */
	for(uint16_t i = 0; i < filterOrder_P; i++) {
		Output += Filter_Buffer_P[i];
	}
	/* Divide */
	Output /= filterOrder_P;
	/* Left Shift */
	for(uint16_t i = 0; i < filterOrder_P; i++) {
		Filter_Buffer_P[i] = Filter_Buffer_P[i+1];
	}
	/* Return filtered value */
	return (uint16_t) Output;
}



