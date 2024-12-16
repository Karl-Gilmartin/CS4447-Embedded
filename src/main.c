#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "dht11.h"
#include "bluetooth.h"
#include "servo.h"
#include "potentiometer.h"
#include "main.h"

// Servo control task declaration (defined elsewhere)
void servo_task(void *param);


// Bluetooth callback for sending data

CircularBuffer dht_buffer;
SemaphoreHandle_t buffer_mutex;

// App main
void app_main() {
    // Initialize NVS (required for Bluetooth)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //state machine
    statemachine_init();


    // Create a queue for servo commands
    command_queue = xQueueCreate(10, sizeof(char[16])); // Ensure queue size matches servo_task
    if (command_queue == NULL) {
        printf("Failed to create command queue\n");
        return;
    }

    init_buffer(&dht_buffer);
    buffer_mutex = xSemaphoreCreateMutex();
    if (buffer_mutex == NULL) {
        printf("Failed to create buffer mutex\n");
        return;
    }

    if (xTaskCreate(dht_task, "DHT Task", 2048, &dht_buffer, 5, NULL) != pdPASS) {
        ESP_LOGE("Main", "Failed to create DHT task");
    }


    ESP_LOGI("Main", "System initialized successfully");

    // Create the servo task
    if (xTaskCreate(servo_task, "Servo Task", 2048, NULL, 5, NULL) != pdPASS) {
        ESP_LOGE("Main", "Failed to create Servo task");
        return;
    }


    // Initialize Bluetooth
    bluetooth_init();


    
    // Initialize ADC
    // potentiometer_init();

    // Configure ISR timer
    // setup_potentiometer_timer();

    // Start the potentiometer task
    // xTaskCreate(potentiometer_task, "Potentiometer Task", 2048, NULL, 5, NULL);

    

}
