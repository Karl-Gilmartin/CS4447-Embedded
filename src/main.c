#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "dht11.h"
#include "bluetooth.h"
#include "servo.h"
#include "potentiometer.h"

// Servo control task declaration (defined elsewhere)
void servo_task(void *param);


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

    // Create a queue for servo commands
    command_queue = xQueueCreate(10, sizeof(char[16])); // Ensure queue size matches servo_task
    if (command_queue == NULL) {
        printf("Failed to create command queue\n");
        return;
    }

    // Create a queue for DHT data
    dht_queue = xQueueCreate(10, sizeof(struct dht_reading));
    if (dht_queue == NULL) {
        printf("Failed to create DHT queue\n");
        return;
    }

    // Create the servo task
    //xTaskCreate(servo_task, "Servo Task", 2048, NULL, 5, NULL);



    // Initialize Bluetooth
    bluetooth_init();
    
    // Initialize ADC
    potentiometer_init();

    // Configure ISR timer
    setup_potentiometer_timer();

    // Start the potentiometer task
    xTaskCreate(potentiometer_task, "Potentiometer Task", 2048, NULL, 5, NULL);

    // Start DHT task
    xTaskCreate(dht_task, "DHT Task", 2048, NULL, 5, NULL);

}
