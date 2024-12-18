#include "bluetooth.h"
#include "esp_bt.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_log.h"


uint8_t ble_addr_type;
void ble_app_advertise(void);

#include <esp_wifi.h>

void get_mac_address(void) {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
void get_bluetooth_mac_address(void) {
    uint8_t mac[6];
    esp_err_t ret = esp_read_mac(mac, ESP_MAC_BT); // Read Bluetooth MAC address
    if (ret == ESP_OK) {
        printf("Bluetooth MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        printf("Failed to read Bluetooth MAC Address\n");
    }
}

// Write data to ESP32 defined as server
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    char command[16];
    strncpy(command, (char *)ctxt->om->om_data, ctxt->om->om_len);
    command[ctxt->om->om_len] = '\0'; // Null-terminate the string

    ESP_LOGI(TAG, "Data from Agent: %s", command); // It will be True or False
    blink_led(YELLOW_LED_GPIO, 1000);


    if (strcmp(command, "OPEN") == 0) {
        statemachine_handle_event("OPEN_WINDOW");
    } else if (strcmp(command, "CLOSE") == 0) {
        statemachine_handle_event("CLOSE_WINDOW");
    } else if (strcmp(command, "MANUAL_MODE") == 0) {
        statemachine_handle_event("MANUAL_MODE");
    } else if (strcmp(command, "AUTO_MODE") == 0) {
        statemachine_handle_event("AUTO_MODE");
    } else {
        ESP_LOGW(TAG, "Unknown command: %s", command);
    }

    return 0;
}


int send_data(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    DhtData data;
    const char *device_name = "Firebeetle"; // Replace with your device name
    size_t name_len = strlen(device_name);

    // Protect buffer access with the mutex
    if (xSemaphoreTake(buffer_mutex, pdMS_TO_TICKS(100))) {
        if (get_from_buffer(&dht_buffer, &data)) {
            // Calculate the total buffer size: 12 bytes for DHT data + device name length + 1 for null terminator
            size_t buffer_size = 12 + name_len + 1;
            uint8_t *buffer = (uint8_t *)malloc(buffer_size);
            if (!buffer) {
                ESP_LOGE(TAG, "Failed to allocate memory for buffer");
                xSemaphoreGive(buffer_mutex);
                os_mbuf_append(ctxt->om, "Error", strlen("Error"));
                return -1;
            }

            // Add the device name to the buffer
            memcpy(buffer, device_name, name_len);

            // Add header
            buffer[name_len + 0] = 'D';
            buffer[name_len + 1] = 'H';
            buffer[name_len + 2] = 'T';

            // Serialize temperature and humidity
            union {
                float value;
                uint8_t bytes[sizeof(float)];
            } temp_union, hum_union;

            temp_union.value = data.temperature;
            hum_union.value = data.humidity;

            memcpy(&buffer[name_len + 3], temp_union.bytes, sizeof(float));
            memcpy(&buffer[name_len + 7], hum_union.bytes, sizeof(float));

            // Calculate checksum
            buffer[name_len + 11] = 0;
            for (int i = 0; i < name_len + 11; i++) {
                buffer[name_len + 11] ^= buffer[i];
            }

            // Send the serialized data to the BLE client
            os_mbuf_append(ctxt->om, buffer, buffer_size);

            ESP_LOGI(TAG, "Sent DHT data: Device=%s, Temp=%.2f°C, Humidity=%.2f%%",
                     device_name, data.temperature, data.humidity);
            blink_led(GREEN_LED_GPIO, 1000);

            free(buffer); // Free allocated memory
        } else {
            ESP_LOGW(TAG, "Buffer is empty, no data to send");
            os_mbuf_append(ctxt->om, "No data", strlen("No data"));
        }
        xSemaphoreGive(buffer_mutex);
    } else {
        ESP_LOGW(TAG, "Failed to acquire mutex for buffer");
        os_mbuf_append(ctxt->om, "Error", strlen("Error"));
    }
    return 0;
}



static int read_window_state(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    const char *state = window_open ? "OPEN" : "CLOSED";
    os_mbuf_append(ctxt->om, state, strlen(state));
    ESP_LOGI(TAG, "Window state read: %s", state);


    if (strcmp(state, "OPEN") == 0) {
        statemachine_handle_event("OPEN_WINDOW");
    } else if (strcmp(state, "CLOSED") == 0) {
        statemachine_handle_event("CLOSE_WINDOW");
    } else {
        ESP_LOGW(TAG, "Unknown window state command: %s", state);
    }

    return 0;
}

static int read_temperature(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
    // Prepare the temperature value as bytes
    uint8_t temperature_bytes[4];
    memcpy(temperature_bytes, &potentiometer_temperature, sizeof(potentiometer_temperature));

    // Send the temperature data
    int rc = os_mbuf_append(ctxt->om, temperature_bytes, sizeof(temperature_bytes));
    if (rc == 0) {
        ESP_LOGI(TAG, "Temperature sent to BLE: %.2f°C", potentiometer_temperature);
        return 0;  // Success
    } else {
        ESP_LOGE(TAG, "Failed to append temperature data. Error code: %d", rc);
        return BLE_ATT_ERR_INSUFFICIENT_RES;  // Error response for insufficient resources
    }
}



// Array of pointers to other service definitions
// UUID - Universal Unique Identifier
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x180), // Service UUID
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(0xFEF4),           // DHT characteristic
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = send_data},
         {.uuid = BLE_UUID16_DECLARE(0xDEAD),           // Write characteristic
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {.uuid = BLE_UUID16_DECLARE(0xBEEF),           // Window state characteristic
          .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
          .access_cb = read_window_state},
         {.uuid = BLE_UUID16_DECLARE(0xC0FF),           // Temperature characteristic
          .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
          .access_cb = read_temperature},
         {0}}},
    {0}};


// BLE event handling
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    // Advertise if connected
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "BLE Connection signal status: %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status == 0) { // Connection successful
            struct ble_gap_conn_desc conn_desc;
            if (ble_gap_conn_find(event->connect.conn_handle, &conn_desc) == 0) {
                ESP_LOGI(TAG, "Connected to device with address: %02X:%02X:%02X:%02X:%02X:%02X",
                        conn_desc.peer_ota_addr.val[0],
                        conn_desc.peer_ota_addr.val[1],
                        conn_desc.peer_ota_addr.val[2],
                        conn_desc.peer_ota_addr.val[3],
                        conn_desc.peer_ota_addr.val[4],
                        conn_desc.peer_ota_addr.val[5]);
            } else {
                ESP_LOGE(TAG, "Failed to find connection details for handle: %d", event->connect.conn_handle);
            }
        } else {
            // Restart advertising if connection failed
            ble_app_advertise();
        }
        break;
    // Advertise again after completion of the event
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Disconnected from device with address: %02X:%02X:%02X:%02X:%02X:%02X",
                 event->disconnect.conn.peer_ota_addr.val[0],
                 event->disconnect.conn.peer_ota_addr.val[1],
                 event->disconnect.conn.peer_ota_addr.val[2],
                 event->disconnect.conn.peer_ota_addr.val[3],
                 event->disconnect.conn.peer_ota_addr.val[4],
                 event->disconnect.conn.peer_ota_addr.val[5]);
        // Restart advertising
        blink_led(RED_LED_GPIO, 10000);
        ble_app_advertise();
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    default:
        break;
    }
    return 0;
}

// Define the BLE connection
void ble_app_advertise(void)
{
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); // Read the BLE device name
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

// The application
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
    ble_app_advertise();                     // Define the BLE connection
}

// The infinite task
void host_task(void *param)
{
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
}

void bluetooth_init()
{
    nvs_flash_init();                          // 1 - Initialize NVS flash using
    // esp_nimble_hci_and_controller_init();      // 2 - Initialize ESP controller
    nimble_port_init();                        // 3 - Initialize the host stack
    ble_svc_gap_device_name_set("Karls BLE"); // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application
    nimble_port_freertos_init(host_task);      // 6 - Run the thread
}