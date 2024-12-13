#include "dht11.h"
#include <freertos/queue.h>


struct dht_reading temp_read() {
    struct dht_reading dht_data; // Structure to hold DHT readings
    dht_data.temperature = -1;
    dht_data.humidity = -1;

    // Read data from the DHT sensor
    dht_result_t res = read_dht_sensor_data(DHT_GPIO_PIN, DHT11, &dht_data);

    if (res != DHT_OK) {
        ESP_LOGW(DHT_TAG, "DHT sensor reading failed"); 
    } else {
        ESP_LOGI(DHT_TAG, "DHT sensor reading: %.2f°C, %.2f%% humidity",dht_data.temperature, dht_data.humidity);
    }

    return dht_data;
}


void dht_task(void *pvParameters) {
    while (1) {
        // Read temperature and humidity
        struct dht_reading data = temp_read();

        // Log valid data
        if (data.temperature != -1 && data.humidity != -1) {
            ESP_LOGI("DHT_Task", "Read Temperature: %.2f°C, Humidity: %.2f%%", data.temperature, data.humidity);
        } else {
            ESP_LOGW("DHT_Task", "Failed to read data from DHT sensor.");
        }

        // Add data to the queue and log the enqueue operation
        if (xQueueSend(dht_queue, &data, 0) == pdPASS) {
            ESP_LOGI("DHT_Task", "Enqueued data: Temperature=%.2f°C, Humidity=%.2f%%", data.temperature, data.humidity);
        } else {
            ESP_LOGW("DHT_Task", "Queue full. Failed to enqueue data.");
        }

        // Wait for 2 seconds before reading again
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

