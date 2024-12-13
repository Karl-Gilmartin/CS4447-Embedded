// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"

// // Define GPIO pins for LEDs
// #define RED_LED_GPIO GPIO_NUM_25
// #define YELLOW_LED_GPIO GPIO_NUM_26
// #define GREEN_LED_GPIO GPIO_NUM_27

// void app_main() {
//     // Configure GPIO pins as outputs
//     gpio_config_t io_conf = {
//         .pin_bit_mask = (1ULL << RED_LED_GPIO) | (1ULL << YELLOW_LED_GPIO) | (1ULL << GREEN_LED_GPIO),
//         .mode = GPIO_MODE_OUTPUT,
//         .intr_type = GPIO_INTR_DISABLE,
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .pull_up_en = GPIO_PULLUP_DISABLE
//     };
//     gpio_config(&io_conf);

//     while (1) {
//         // Turn on the red LED
//         gpio_set_level(RED_LED_GPIO, 1);
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second

//         // Turn off the red LED and turn on the yellow LED
//         gpio_set_level(RED_LED_GPIO, 0);
//         gpio_set_level(YELLOW_LED_GPIO, 1);
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second

//         // Turn off the yellow LED and turn on the green LED
//         gpio_set_level(YELLOW_LED_GPIO, 0);
//         gpio_set_level(GREEN_LED_GPIO, 1);
//         vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second

//         // Turn off the green LED
//         gpio_set_level(GREEN_LED_GPIO, 0);
//     }
// }
