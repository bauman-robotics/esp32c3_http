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


//#define RECEIVE_ANSWER_ENABLE 
//=================================


