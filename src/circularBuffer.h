#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <stdbool.h>
#include "esp_log.h"


#define BUFFER_SIZE 10

// Structure to hold DHT data
typedef struct {
    float temperature;
    float humidity;
} DhtData;

// Circular buffer structure
typedef struct {
    DhtData buffer[BUFFER_SIZE];
    int head;
    int tail;
    int count;
} CircularBuffer;

// Declare mutex for thread safety
extern SemaphoreHandle_t buffer_mutex;

// Function declarations
void init_buffer(CircularBuffer *cb);
bool is_buffer_full(CircularBuffer *cb);
bool is_buffer_empty(CircularBuffer *cb);
void add_to_buffer(CircularBuffer *cb, DhtData data);
bool get_from_buffer(CircularBuffer *cb, DhtData *data);

#endif // CIRCULARBUFFER_H
