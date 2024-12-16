#include "circularBuffer.h"


void init_buffer(CircularBuffer *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
}

bool is_buffer_full(CircularBuffer *cb) {
    return cb->count == BUFFER_SIZE;
}

bool is_buffer_empty(CircularBuffer *cb) {
    return cb->count == 0;
}

void add_to_buffer(CircularBuffer *cb, DhtData data) {
    if (is_buffer_full(cb)) {
        // Overwrite the oldest data
        ESP_LOGW("CircularBuffer", "Buffer is full, overwriting data");
        cb->tail = (cb->tail + 1) % BUFFER_SIZE;
    } else {
        cb->count++;
    }

    cb->buffer[cb->head] = data;
    cb->head = (cb->head + 1) % BUFFER_SIZE;
}

bool get_from_buffer(CircularBuffer *cb, DhtData *data) {
    if (is_buffer_empty(cb)) {
        return false; // Buffer is empty
    }

    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) % BUFFER_SIZE;
    cb->count--;

    return true;
}
