#include "servo.h"
#include <stdio.h>
#include <string.h>

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

// Task to control the servo motor based on commands
void servo_task(void *param) {
    char command[16];

    // Configure LEDC PWM for servo control
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // PWM resolution
        .freq_hz = SERVO_FREQ,               // Frequency (50Hz for servos)
        .speed_mode = LEDC_LOW_SPEED_MODE,   // Low-speed mode
        .timer_num = LEDC_TIMER_0            // Use timer 0
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 0,
        .gpio_num   = SERVO_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel  = LEDC_TIMER_0
    };
    ledc_channel_config(&ledc_channel);

    while (1) {
        // Wait for a command from the queue
        if (xQueueReceive(command_queue, &command, portMAX_DELAY)) {
            if (strcmp(command, OPEN_WINDOW) == 0) {
                window_open = true;
                printf("Opening the window (180 degrees)\n");
                ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, servo_angle_to_duty(180));
                ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            } else if (strcmp(command, CLOSE_WINDOW) == 0) {
                window_open = false;
                printf("Closing the window (0 degrees)\n");
                ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, servo_angle_to_duty(0));
                ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            } else {
                printf("Unknown command: %s\n", command);
            }
        }
    }
}
