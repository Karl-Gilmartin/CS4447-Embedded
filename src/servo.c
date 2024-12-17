#include "servo.h"


// Define the queue handle
QueueHandle_t command_queue;

// Define `window_open`
volatile bool window_open = false;

// Function to convert angle to pulse width
uint32_t servo_angle_to_duty(int angle) {
    uint32_t pulsewidth = SERVO_MIN_PULSEWIDTH +
                          ((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * angle) / SERVO_MAX_DEGREE;
    return (pulsewidth * 8192) / 20000; // Convert to duty cycle for 50Hz
}

void servo_init() {
    // Configure LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .freq_hz          = 50,  // 50Hz frequency for servo motors
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Configure LEDC channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO_GPIO,  // Replace with the GPIO number connected to your servo
        .duty           = 0,           // Initial duty cycle
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ESP_LOGI("Servo", "LEDC initialized for servo control.");
}

float get_current_temperature() {
    DhtData data;
    float current_temperature = -1.0; // Default value if no data is available
    // ESP_LOGW("CircularBuffer X Servo", "trying to read");

    // Protect access to the circular buffer
    if (xSemaphoreTake(buffer_mutex, pdMS_TO_TICKS(100))) {
        if (get_from_buffer(&dht_buffer, &data)) {
            current_temperature = data.temperature;
            ESP_LOGI("SERVO X CB", "Current temperature: %.2f°C", current_temperature);
        } else {
            ESP_LOGW("CircularBuffer", "No data available in buffexr");
        }
        xSemaphoreGive(buffer_mutex);
    } else {
        ESP_LOGW("CircularBuffer", "Failed to acquire buffer mutex");
    }

    return current_temperature;
}

// Task to control the servo motor based on commands
void servo_task(void *param) {
    servo_init();
    static bool window_is_open = false;

    while (1) {
        SystemState state = statemachine_get_state();

        switch (state) {
            case STATE_IDLE:
                ESP_LOGI("Servo", "System is idle.");
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;

            case STATE_MANUAL:
                if (statemachine_get_window_state() == WINDOW_OPEN && !window_is_open) {
                    ESP_LOGI("Servo", "Manual: Opening window");
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, servo_angle_to_duty(180));
                    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                    window_is_open = true;
                } else if (statemachine_get_window_state() == WINDOW_CLOSED && window_is_open) {
                    ESP_LOGI("Servo", "Manual: Closing window");
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, servo_angle_to_duty(0));
                    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                    window_is_open = false;
                }
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;

            case STATE_AUTO:
                float current_temperature = get_current_temperature();
                if (current_temperature > 20.0 && !window_is_open) {
                    ESP_LOGI("Servo", "Auto: Temperature high (%.2f°C). Opening window.", current_temperature);
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, servo_angle_to_duty(180));
                    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                    window_is_open = true;
                } else if (current_temperature < 15.0 && window_is_open) {
                    ESP_LOGI("Servo", "Auto: Temperature low (%.2f°C). Closing window.", current_temperature);
                    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, servo_angle_to_duty(0));
                    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                    window_is_open = false;
                }
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;

            case STATE_ERROR:
                ESP_LOGE("Servo", "Error state. Halting operations.");
                vTaskDelay(pdMS_TO_TICKS(2000));
                break;

            default:
                ESP_LOGE("Servo", "Unknown state.");
                break;
        }
    }
}
