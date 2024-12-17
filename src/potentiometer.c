#include "potentiometer.h"

// Global variables
volatile float potentiometer_temperature = 0.0;
volatile bool potentiometer_changed_flag = false;  // ISR flag

void IRAM_ATTR potentiometer_isr_handler(void *arg) {
    static int last_value = -1;  // Store the last ADC value
    int current_value = adc1_get_raw(POTENTIOMETER_ADC_CHANNEL);  // Read ADC value

    // Check for significant change or threshold crossings
    if (last_value == -1 || abs(current_value - last_value) > CHANGE_THRESHOLD) {
        potentiometer_changed_flag = true;  // Set flag for the task

        // Log details (Use ESP_EARLY_LOG for ISR-safe logging)
        if (current_value > THRESHOLD_HIGH && last_value <= THRESHOLD_HIGH) {
            ESP_EARLY_LOGW(TAG, "ISR: High threshold crossed! ADC=%d (Prev=%d)", current_value, last_value);
        } else if (current_value < THRESHOLD_LOW && last_value >= THRESHOLD_LOW) {
            ESP_EARLY_LOGW(TAG, "ISR: Low threshold crossed! ADC=%d (Prev=%d)", current_value, last_value);
        } else {
            ESP_EARLY_LOGI(TAG, "ISR: Significant ADC change detected. ADC=%d (Prev=%d)", current_value, last_value);
        }
    }

    last_value = current_value;  // Update last value
}


void potentiometer_init() {
    // Configure ADC for potentiometer
    adc1_config_width(ADC_WIDTH_BIT_12);  // 12-bit resolution
    adc1_config_channel_atten(POTENTIOMETER_ADC_CHANNEL, ADC_ATTEN_DB_11);  // Full voltage range

    // Configure timer for ISR
    const esp_timer_create_args_t timer_args = {
        .callback = &potentiometer_isr_handler,  // Attach ISR handler
        .name = "potentiometer_timer"
    };

    esp_timer_handle_t timer;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, 100000));  // Trigger every 100ms

    ESP_LOGD(TAG, "Potentiometer initialized and timer configured.");
}

void potentiometer_task(void *pvParameter) {
    ESP_LOGD(TAG, "Potentiometer task started.");
    while (1) {
        // Process only if ISR sets the flag
        if (potentiometer_changed_flag) {
            potentiometer_changed_flag = false;  // Clear the flag

            // Read ADC value and update temperature
            int adc_value = adc1_get_raw(POTENTIOMETER_ADC_CHANNEL);
            potentiometer_temperature = ((adc_value / 4095.0) * 20.0) + 10.0;  // Scale to 10°C - 30°C

            ESP_LOGD(TAG, "Potentiometer updated: ADC=%d -> Temperature=%.2f°C", 
                     adc_value, potentiometer_temperature);
        }

        // Add a small delay to avoid high CPU usage
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
