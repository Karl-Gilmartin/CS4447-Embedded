#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "statemachine.h"

// Define GPIO pin for the servo signal
#define SERVO_GPIO 18  // Use any PWM-capable GPIO pin

// Servo PWM parameters
#define SERVO_MIN_PULSEWIDTH 500  // Minimum pulse width in microseconds (0°)
#define SERVO_MAX_PULSEWIDTH 2500 // Maximum pulse width in microseconds (180°)
#define SERVO_MAX_DEGREE 180      // Maximum degree of rotation of servo
#define SERVO_FREQ 50             // Servo PWM frequency (50Hz)

// Command definitions
#define OPEN_WINDOW "OPEN"
#define CLOSE_WINDOW "CLOSE"

// Queue handle for sending commands
extern QueueHandle_t command_queue;  // Extern to share across files

// Function to convert angle to pulse width
uint32_t servo_angle_to_duty(int angle);

extern volatile bool window_open;

// Task to control the servo motor based on commands
void servo_task(void *param);

#endif // SERVO_CONTROL_H
