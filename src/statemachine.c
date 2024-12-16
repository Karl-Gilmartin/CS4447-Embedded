#include "statemachine.h"


// Static variables for states
static SystemState current_state = STATE_IDLE;
static WindowState window_state = WINDOW_CLOSED;

static const char *TAG = "StateMachine";

// Initialize the state machine
void statemachine_init(void) {
    current_state = STATE_IDLE;
    window_state = WINDOW_CLOSED;
    ESP_LOGI(TAG, "State machine initialized: IDLE, Window: CLOSED");
}

// Handle events and update state
void statemachine_handle_event(const char *command) {
    if (strcmp(command, "AUTO_MODE") == 0) {
        current_state = STATE_AUTO;
        ESP_LOGI(TAG, "Transition to AUTO mode");
    } else if (strcmp(command, "MANUAL_MODE") == 0) {
        current_state = STATE_MANUAL;
        ESP_LOGI(TAG, "Transition to MANUAL mode");
    } else if (strcmp(command, "RESET") == 0) {
        current_state = STATE_IDLE;
        ESP_LOGI(TAG, "System reset to IDLE state");
    } else if (strcmp(command, "OPEN_WINDOW") == 0) {
        window_state = WINDOW_OPEN;
        ESP_LOGI(TAG, "Window state set to OPEN");
        if (current_state != STATE_MANUAL) {
            current_state = STATE_MANUAL;
            ESP_LOGI(TAG, "Transition to MANUAL mode");
        }
    } else if (strcmp(command, "CLOSE_WINDOW") == 0) {
        window_state = WINDOW_CLOSED;
        ESP_LOGI(TAG, "Window state set to CLOSED");
        if (current_state != STATE_MANUAL) {
            current_state = STATE_MANUAL;
            ESP_LOGI(TAG, "Transition to MANUAL mode");
        }
    } else if (strcmp(command, "ERROR") == 0) {
        current_state = STATE_ERROR;
        ESP_LOGW(TAG, "Transition to ERROR state");
    } else {
        ESP_LOGW(TAG, "Unknown command: %s", command);
    }
}


// Set State
void statemachine_set_state(SystemState state) {
    current_state = state;
}

// Get the current system state
SystemState statemachine_get_state(void) {
    return current_state;
}

// Get the current window state
WindowState statemachine_get_window_state(void) {
    return window_state;
}

// Set the window state (e.g., called by servo or BLE task)
void statemachine_set_window_state(WindowState state) {
    window_state = state;
    ESP_LOGI(TAG, "Window state updated: %s", state == WINDOW_OPEN ? "OPEN" : "CLOSED");
}
