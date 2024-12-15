#include "potentiometer.h"

volatile float potentiometer_temperature = 0.0;

volatile bool potentiometer_threshold_crossed = false;

void potentiometer_init()
{
    // Configure ADC width (resolution) and attenuation
    adc1_config_width(ADC_WIDTH_BIT_12);                      // 12-bit resolution (0-4095)
    adc1_config_channel_atten(POTENTIOMETER_ADC_CHANNEL, ADC_ATTEN_DB_11); // Full voltage range

    ESP_LOGI(TAG, "Potentiometer initialized.");
}


void IRAM_ATTR potentiometer_isr_handler(void *arg)
{
    static int last_value = -1;
    int value = adc1_get_raw(POTENTIOMETER_ADC_CHANNEL); // Read ADC value

    if ((value > THRESHOLD_HIGH && last_value <= THRESHOLD_HIGH) ||
        (value < THRESHOLD_LOW && last_value >= THRESHOLD_LOW))
    {
        // Set flag for the main task
        potentiometer_threshold_crossed = true;

        // Log ISR activity
        ESP_EARLY_LOGI(TAG, "Threshold crossed! Value: %d", value);
    }

    last_value = value; // Update the last ADC value
}


void setup_potentiometer_timer()
{
    // Create a timer configuration
    const esp_timer_create_args_t timer_args = {
        .callback = &potentiometer_isr_handler,
        .name = "potentiometer_timer"
    };

    esp_timer_handle_t timer;
    esp_timer_create(&timer_args, &timer);

    // Start the timer with a periodic interval (e.g., 100ms)
    esp_timer_start_periodic(timer, 100000); // 100ms interval
    ESP_LOGI(TAG, "Potentiometer ISR timer configured.");
}



void potentiometer_task(void *pvParameter)
{
    while (1)
    {
        // Check if the ISR flag is set
        if (potentiometer_threshold_crossed)
        {
            potentiometer_threshold_crossed = false; // Clear the flag

            // Read the current ADC value
            int raw_value = adc1_get_raw(POTENTIOMETER_ADC_CHANNEL);
            float temperature = ((raw_value / 4095.0) * 20.0) + 10.0;
            potentiometer_temperature = temperature;

            // Perform an action based on the ADC value
            if (raw_value > THRESHOLD_HIGH)
            {
                ESP_LOGI(TAG, "High threshold crossed! Temp: %.2f°C", temperature);
                // Add logic to adjust the servo or perform another action
            }
            else if (raw_value < THRESHOLD_LOW)
            {
                ESP_LOGI(TAG, "Low threshold crossed! Temp: %.2f°C", temperature);
                // Add logic to adjust the servo or perform another action
            }
        }

        // Delay for task responsiveness
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
