#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "dht11.h"
#include "bluetooth.h"


// Bluetooth callback for sending data

QueueHandle_t dht_queue;

// App main
void app_main() {
    // Initialize NVS (required for Bluetooth)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Create a queue for DHT data
    dht_queue = xQueueCreate(10, sizeof(struct dht_reading));
    if (dht_queue == NULL) {
        printf("Failed to create DHT queue\n");
        return;
    }

    // Initialize Bluetooth
    bluetooth_init();

    // Start DHT task
    xTaskCreate(dht_task, "DHT Task", 2048, NULL, 5, NULL);

    // The BLE logic automatically uses `send_data` through the GATT service definition
    // No need to start `send_data` as a task.
}
