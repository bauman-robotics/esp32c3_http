#include "variables.h"


variables var;

// Очередь для передачи данных

QueueHandle_t xQueueSignalData;
QueueHandle_t xQueueSignalReady;