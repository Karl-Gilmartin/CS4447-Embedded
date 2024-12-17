#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include <esp_timer.h>
#include "esp_log.h"
static const char *TAG = "Potentiometer";

#define POTENTIOMETER_ADC_CHANNEL ADC1_CHANNEL_0  // GPIO36 (A0) on FireBeetle
#define CHANGE_THRESHOLD 80                     // Minimum ADC change to trigger ISR
#define THRESHOLD_HIGH 3000                      // Example high threshold
#define THRESHOLD_LOW 1000                       // Example low threshold

extern volatile float potentiometer_temperature;  // Target temperature
extern volatile bool potentiometer_changed_flag;  // Flag set by ISR

void potentiometer_init();
void potentiometer_task(void *pvParameter);

#endif // POTENTIOMETER_H
