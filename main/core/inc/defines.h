#pragma once
#include "main.h"

//=== Mode ========================

#define LED_BLINK_ENABLE
//#define SEND_UART_SIN_ENABLE
//#define POST_REQUEST_ENABLE
#define SOCKET_CLIENT_ENABLE
#define USB_SERIAL_JTAG_TASK_ENABLE


//=== T-me Params =================
#define UART_SIN_SEND_PERIOD_MS  (10)
#define POST_REQUEST_PERIOD_MS (3000)
#define SOCKET_SEND_PERIOD_MS    (10)
//=========================================

//=== Выберите целевой сервер ===
#define DEST_SERVER  (SERVER_YPC)
//#define DEST_SERVER (SERVER_LENOVO_UNDER_MOBILE_NET)
//#define DEST_SERVER (SERVER_LENOVO_UNDER_XIAOMI)
//#define DEST_SERVER (SERVER_LENOVO_HOT_SPOT)
//===============================

//=== Выбор порта подключения === 
#define SOCKET_PORT (18082)

//========================================

#define SIGNAL_GEN_TASK_EN

#define DATA_PREFIX_INT   "d "
#define DATA_PREFIX_FLOAT "f "
#define NUM_ELEMENT_IN_PACKET       (10)
#define MAX_NUM_ELEMENT_IN_PACKET   (1000)

#define SOCKET_BUF_TX_SIZE    (1500)

#define BINARY_PACKET_INT_KEY (2255)
#define BINARY_PACKET_FLOAT_KEY (2233)

#define BINARY_PACKET 
#define DATA_TYPE_FLOAT 

//#define DEBUG_LOG

//#define ESP32_C3

#define LOG_TASK_ENABLE
#define LOG_TASK_PERIOD_MS (1000)

#define INA226_ENABLE
#define I2C_SDA_PIN (21)
#define I2C_SCL_PIN (22)

#define INA226_CFG_REG_VAL (0x4127) // Average over 4 Samples
//#define INA226_CFG_REG_VAL (0x4527) // Average over 16 Samples
//#define INA226_CFG_REG_VAL (0x4f27) // Average over 1024 Samples

//=== input ====
const uint16_t   MAX_EXPECTED_CURRENT_mA   = 1000; // mA
const float INA226_R_SHUNT_Om              = 0.05;
//=== Output ===
const float      INA226_LSB_mA             = (float)MAX_EXPECTED_CURRENT_mA  / 32768 ; // 2**15 = 32768
const float      INA226_LSB_mkA            = INA226_LSB_mA * 1000;  // not used
const uint16_t   INA226_CALIBRATION_VAL    = (uint16_t)( 5.12 / (INA226_LSB_mA  * INA226_R_SHUNT_Om));  // not used
//==============

//#define GET_CURRENT_DEFAULT

//#define FILTER_V_I_P_ENABLE 
#define FILTER_ORDER_V (100)
#define FILTER_ORDER_I (100)
#define FILTER_ORDER_P (100)

