#include "led.h"

void init_leds() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RED_LED_GPIO) | (1ULL << YELLOW_LED_GPIO) | (1ULL << GREEN_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}


void blink_led(gpio_num_t led_gpio, int duration_ms) {
    gpio_set_level(led_gpio, 1); // Turn on the LED
    vTaskDelay(pdMS_TO_TICKS(duration_ms)); // Delay
    gpio_set_level(led_gpio, 0); // Turn off the LED
}
