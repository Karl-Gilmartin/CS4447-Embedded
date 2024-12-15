// potentiometer.h
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

#define THRESHOLD_HIGH 3000                     // Example high threshold
#define THRESHOLD_LOW 1000                     // Example low threshold

extern volatile bool potentiometer_threshold_crossed;

void potentiometer_task(void *pvParameter);
void potentiometer_init();
void setup_potentiometer_timer();

extern volatile float potentiometer_temperature; 

#endif // POTENTIOMETER_H