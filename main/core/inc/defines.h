#pragma once
#include "main.h"

//=== Mode ========================

#define LED_BLINK_ENABLE
//#define SEND_UART_SIN_ENABLE
//#define POST_REQUEST_ENABLE
#define SOCKET_CLIENT_ENABLE

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


