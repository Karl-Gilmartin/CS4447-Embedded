# ifndef LED_H
# define LED_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

// Define GPIO pins for LEDs
#define RED_LED_GPIO GPIO_NUM_25
#define YELLOW_LED_GPIO GPIO_NUM_26
#define GREEN_LED_GPIO GPIO_NUM_27

void init_leds();
void blink_led(gpio_num_t led_gpio, int duration_ms);


#endif
