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
#include "bluetoothsetup.h"
#include "dht11.h"

void app_main();

#endif // MAIN_H


