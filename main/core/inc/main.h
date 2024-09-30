#pragma once
#include "variables.h"

//=== SOCKET Params ===============
// Определите макросы для серверов
#define SERVER_YPC                     (1)
#define SERVER_LENOVO_UNDER_MOBILE_NET (2)
#define SERVER_LENOVO_UNDER_XIAOMI     (3)
#define SERVER_LENOVO_HOT_SPOT         (4)

//=== LEDS States =========================
#define LEDS_NO_CONNECT_STATE          (0)
#define LEDS_GOT_IP_STATE              (1)
#define LEDS_CONNECT_TO_SERVER_STATE   (2)


#define SIN_PERIOD_MS      1000 // 1 second for a full sine wave cycle
#define SIN_VALUES_COUNT   100  // 100 values per period
#define AMPLITUDE          1000 // Amplitude from 0 to 1000

