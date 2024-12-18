# ifndef DHT11_H
# define DHT11_H

#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht_espidf.h"
#include "potentiometer.h"
#include "config.h"
#include "circularBuffer.h"
#include <freertos/queue.h>

#define DHT_TAG "DHT_11"

// Declare the temp_read function prototype
struct dht_reading temp_read();

void dht_task(void *pvParameters);

#endif // DHT11_H

