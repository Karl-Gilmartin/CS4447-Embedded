#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <stdbool.h>
#include <string.h>
#include "esp_log.h"

// Define system states
typedef enum {
    STATE_IDLE,
    STATE_MANUAL,
    STATE_AUTO,
    STATE_ERROR
} SystemState;

// Window states
typedef enum {
    WINDOW_CLOSED,
    WINDOW_OPEN
} WindowState;

// Initialize the state machine
void statemachine_init(void);

// Handle events
void statemachine_handle_event(const char *command);

// Get current system state
SystemState statemachine_get_state(void);

void statemachine_set_state(SystemState state);

// Get current window state
WindowState statemachine_get_window_state(void);

// Set window state
void statemachine_set_window_state(WindowState state);

#endif // STATEMACHINE_H
