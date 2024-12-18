#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include <esp_timer.h>
#include "esp_log.h"

// Logging tag
static const char *TAG = "Potentiometer";

// GPIO and ADC configuration
#define POTENTIOMETER_ADC_CHANNEL ADC1_CHANNEL_0  // GPIO36 (A0) on FireBeetle
#define CHANGE_THRESHOLD 80                      // Minimum ADC change to trigger ISR
#define THRESHOLD_HIGH 3000                      // High threshold
#define THRESHOLD_LOW 1000                       // Low threshold

// Update thresholds
#define TEMPERATURE_CHANGE_THRESHOLD 0.5         // Minimum temperature change to send updates
#define TEMPERATURE_UPDATE_INTERVAL 30000        // Minimum interval between updates (in ms)

// Global variables
extern volatile float potentiometer_temperature;  // Current potentiometer temperature value
extern volatile bool potentiometer_changed_flag;  // Flag to indicate change in temperature

// Function prototypes
void potentiometer_init();                         // Initialize the potentiometer
void potentiometer_task(void *pvParameter);        // Task to monitor potentiometer changes

#endif // POTENTIOMETER_H
