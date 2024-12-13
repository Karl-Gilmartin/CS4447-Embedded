#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"

#define POTENTIOMETER_ADC_CHANNEL ADC1_CHANNEL_0  // GPIO36 (A0) on FireBeetle

void potentiometer_task(void *pvParameter)
{
    while (1)
    {
        // Read the raw ADC value
        int raw_value = adc1_get_raw(POTENTIOMETER_ADC_CHANNEL);

        float temperature = ((raw_value / 4095.0) * 20.0) + 10.0;

        // Print the raw value and voltage
        printf("Raw Value: %d\tTemperature: %.2f °C\n", raw_value, temperature);


        vTaskDelay(pdMS_TO_TICKS(500));  // Delay 500ms
    }
}

// void app_main()
// {
//     // Configure the ADC channel for the potentiometer
//     adc1_config_width(ADC_WIDTH_BIT_12);  // Set ADC resolution to 12 bits
//     adc1_config_channel_atten(POTENTIOMETER_ADC_CHANNEL, ADC_ATTEN_DB_11);  // 0-3.3V range

//     // Create a FreeRTOS task to read the potentiometer
//     xTaskCreate(potentiometer_task, "potentiometer_task", 2048, NULL, 5, NULL);
// }
