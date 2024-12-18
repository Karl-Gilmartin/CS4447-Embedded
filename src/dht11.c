#include "dht11.h"



struct dht_reading temp_read() {
    struct dht_reading dht_data = { .temperature = -1, .humidity = -1 };

    int retries = 3;
    while (retries-- > 0) {
        dht_result_t res = read_dht_sensor_data(DHT_GPIO_PIN, DHT11, &dht_data);
        if (res == DHT_OK) {
            if (dht_data.temperature >= -40 && dht_data.temperature <= 80 &&
                dht_data.humidity >= 0 && dht_data.humidity <= 100) {
                return dht_data;
            } else {
                ESP_LOGW(DHT_TAG, "Out-of-range reading: %.2f°C, %.2f%%", dht_data.temperature, dht_data.humidity);
            }
        } else {
            ESP_LOGW(DHT_TAG, "Read failed. Retrying...");
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Wait before retrying
    }

    ESP_LOGE(DHT_TAG, "Failed to read valid data from DHT sensor");
    return dht_data;
}



void dht_task(void *pvParameters) {
    CircularBuffer *dht_buffer = (CircularBuffer *)pvParameters;

    while (1) {
        struct dht_reading data = temp_read();

        if (data.temperature != -1 && data.humidity != -1) {
            DhtData new_data = { .temperature = data.temperature, .humidity = data.humidity };

            // Protect buffer with mutex
            if (xSemaphoreTake(buffer_mutex, pdMS_TO_TICKS(100))) {
                add_to_buffer(dht_buffer, new_data);
                ESP_LOGI("DHT_Task", "Added to buffer: %.2f°C, %.2f%%", new_data.temperature, new_data.humidity);
                xSemaphoreGive(buffer_mutex);
            } else {
                ESP_LOGW("DHT_Task", "Failed to acquire mutex for buffer.");
            }
        } else {
            ESP_LOGW("DHT_Task", "Failed to read data from DHT sensor.");
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Wait for 2 seconds
    }
}



