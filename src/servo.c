// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/ledc.h"
// #include "esp_err.h"

// // Define GPIO pin for the servo signal
// #define SERVO_GPIO 18  // Use any PWM-capable GPIO pin

// // Servo PWM parameters
// #define SERVO_MIN_PULSEWIDTH 500  // Minimum pulse width in microseconds (0°)
// #define SERVO_MAX_PULSEWIDTH 2500 // Maximum pulse width in microseconds (180°)
// #define SERVO_MAX_DEGREE 180      // Maximum degree of rotation

// // Function to convert angle to pulse width
// uint32_t servo_angle_to_duty(int angle) {
//     uint32_t pulsewidth = SERVO_MIN_PULSEWIDTH +
//                           ((SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH) * angle) / SERVO_MAX_DEGREE;
//     return (pulsewidth * 8192) / 20000; // Convert to duty cycle for 50Hz
// }

// void app_main() {
//     // Configure LEDC PWM for servo control
//     ledc_timer_config_t ledc_timer = {
//         .duty_resolution = LEDC_TIMER_13_BIT, // PWM resolution
//         .freq_hz = 50,                        // Frequency (50Hz for servos)
//         .speed_mode = LEDC_LOW_SPEED_MODE,    // Low-speed mode
//         .timer_num = LEDC_TIMER_0             // Use timer 0
//     };
//     ledc_timer_config(&ledc_timer);

//     ledc_channel_config_t ledc_channel = {
//         .channel    = LEDC_CHANNEL_0,
//         .duty       = 0,
//         .gpio_num   = SERVO_GPIO,
//         .speed_mode = LEDC_LOW_SPEED_MODE,
//         .timer_sel  = LEDC_TIMER_0
//     };
//     ledc_channel_config(&ledc_channel);

//     // Move the servo to different positions
//     while (1) {
//         // Move to 0 degrees
//         printf("Moving to 0 degrees\n");
//         ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, servo_angle_to_duty(0));
//         ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 1 second

//         // Move to 90 degrees
//         printf("Moving to 90 degrees\n");
//         ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, servo_angle_to_duty(90));
//         ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 1 second

//         // Move to 180 degrees
//         printf("Moving to 180 degrees\n");
//         ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, servo_angle_to_duty(180));
//         ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for 1 second
//     }
// }
