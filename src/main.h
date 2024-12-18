# ifndef MAIN_H
# define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "esp_err.h"
#include "dht11.h"
#include "servo.h"
#include "circularBuffer.h"
#include "statemachine.h"
#include "led.h"

extern CircularBuffer dht_buffer;
extern SemaphoreHandle_t buffer_mutex;

void app_main();

#endif // MAIN_H


